#include "app-context.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <window.h>

#include "sigc++/adaptors/track_obj.h"

AppContext::AppContext(ui::ProgressWindow &app_window,
                       ui::BoardWidget &board_widget, BoardManager &manager)
    : m_app_window(app_window),
      m_board_widget(board_widget),
      m_manager(manager) {
    m_load_board_dispatcher.connect(
        sigc::mem_fun(*this, &AppContext::on_session_loaded));
    m_save_board_dispatcher.connect(
        sigc::mem_fun(*this, &AppContext::on_session_saved));

    m_board_widget.signal_cardlist_added().connect(
        sigc::mem_fun(*this, &AppContext::register_cardlist_container_update));
}

void AppContext::open_session(const std::string &filename) {
    spdlog::get("app")->debug(
        "[AppContext.open_session] Dispatch board session starter thread");
    m_board_load_thread = std::thread{[this, filename]() {
        try {
            this->m_current_board = this->m_manager.local_open(filename);
        } catch (std::invalid_argument &err) {
            this->m_current_board = nullptr;
        }
        // Calls on_board_loaded
        m_load_board_dispatcher.emit();
    }};
}

void AppContext::close_session() {
    if (m_current_board) {
        if (m_board_save_thread.joinable()) {
            spdlog::get("app")->debug(
                "[AppContext] Saver worker thread still running. Joining");
            m_board_save_thread.join();
        } else {
            m_manager.local_save(m_current_board);
        }
        m_manager.local_close(m_current_board);

        spdlog::get("app")->info("Closed Kanban board session (\"{}\")",
                                 m_current_board->get_name());

        reset_session_state();

        // Any pending loading operations will be immediatelly cancelled
        m_session_flags[Status::LOADING] = false;
        if (!m_board_widget.empty()) {
            m_session_flags[Status::CLEARING] = true;
            Glib::signal_idle().connect(
                sigc::mem_fun(*this, &AppContext::idle_clear_session),
                Glib::PRIORITY_LOW);
        }
    }
}

void AppContext::reset_session_state() {
    m_next_card_i = 0;
    m_cardlist_i = 0;
    m_cards.clear();
    m_idle_load_session_cnn.disconnect();
    m_timeout_save_cnn.disconnect();
    m_timeout_cards_update_cnn.disconnect();
    m_board_widget.clear();
    m_current_board = nullptr;
}

void AppContext::on_session_loaded() {
    if (m_board_load_thread.joinable()) {
        m_board_load_thread.join();
    }

    if (m_current_board) {
        spdlog::get("app")->info("Kanban Board Session (\"{}\") started",
                                 m_current_board->get_name());
        m_session_flags[Status::LOADING] = true;

        m_app_window.set_title(m_current_board->get_name());
        m_app_window.on_board_view();
        m_board_widget.set(m_current_board);

        m_idle_load_session_cnn = Glib::signal_idle().connect(
            sigc::mem_fun(*this, &AppContext::idle_load_session),
            Glib::PRIORITY_LOW);
        m_timeout_save_cnn = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_save_session),
            AppContext::SAVE_INTERVAL);
        m_timeout_cards_update_cnn = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_update_cards),
            AppContext::UPDATE_INTERVAL);
    } else {
        // Board has failed to load. Go back to previous state
        spdlog::get("app")->warn(
            "[AppContext.on_session_loaded] Board session initialization has "
            "failed");

        m_session_flags[Status::LOADING] = false;
        m_session_flags[Status::BUSY] = false;

        Gtk::AlertDialog::create(_("It was not possible to load this board"))
            ->show(m_app_window);
        m_app_window.on_main_menu();
    }
}

void AppContext::on_session_saved() {
    if (m_board_save_thread.joinable()) {
        m_board_save_thread.join();
        spdlog::get("app")->debug(
            "[AppContext.on_session_saved] Joining save worker thread");
    }

    spdlog::get("app")->info("Changes made to Board (\"{}\") saved",
                             m_current_board->get_name());
}

bool AppContext::idle_load_session() {
    if (!m_current_board) {
        spdlog::get("app")->error(
            "[AppContext.idle_load_session] Current board is invalid. Stopping "
            "loading task");
        m_session_flags[Status::LOADING] = false;
        return false;
    }

    if (m_session_flags[Status::CLEARING] && m_session_flags[Status::LOADING]) {
        spdlog::get("app")->warn(
            "[AppContext.idle_load_session] Board view is not clean. Current "
            "session (\"{}\") is waiting",
            m_current_board->get_name());
        return true;
    }

    const auto &data = m_current_board->container().get_data();
    if (m_cardlist_i > (data.size() - 1) || data.empty()) {
        m_session_flags[Status::LOADING] = false;

        spdlog::get("app")->debug(
            "[AppContext.idle_load_session] Session (\"{}\") has been fully "
            "loaded",
            m_current_board->get_name());
        return false;
    }

    ui::CardlistWidget *cardlist_widget = Gtk::make_managed<ui::CardlistWidget>(
        m_board_widget, data[m_cardlist_i]);
    m_board_widget.append(*cardlist_widget);

    for (const auto &card : cardlist_widget->cardlist()->container()) {
        ui::CardWidget *card_widget = Gtk::make_managed<ui::CardWidget>(card);
        cardlist_widget->append(*card_widget);
    }
    m_cardlist_i++;

    m_session_flags[Status::LOADING] = true;
    m_session_flags[Status::BUSY] = true;
    return true;
}

bool AppContext::idle_clear_session() {
    auto cardlist_widget = m_board_widget.pop();
    if (!cardlist_widget) {
        m_session_flags[Status::CLEARING] = false;
        m_session_flags[Status::BUSY] = false;

        spdlog::get("app")->debug(
            "[AppContext.idle_clear_session] Board view is free");
        return false;
    }

    m_session_flags[Status::BUSY] = true;
    return true;
}

bool AppContext::timeout_save_session() {
    if (!m_current_board) {
        // There is no board to save, stop this timeout procedure
        spdlog::get("app")->error(
            "[AppContext.timeout_save_session] Current board session is "
            "invalid and cannot be saved. Stopping saving task");
        return false;
    } else if (m_current_board->modified()) {
        if (m_board_save_thread.joinable()) {
            spdlog::get("app")->debug(
                "[AppContext.timeout_save_session] Cannot schedule new saver "
                "thread. Saver thread is still joinable");
            return true;
        } else {
            m_board_save_thread = std::thread{[this]() {
                m_manager.local_save(m_current_board);
                m_save_board_dispatcher.emit();
            }};
            spdlog::get("app")->debug(
                "[AppContext.timeout_save_session] Saver worker thread has "
                "been scheduled");
            return true;
        }
    } else {
        spdlog::get("app")->debug(
            "[AppContext.timeout_save_session] No modifications registered. "
            "Don't schedule a saver "
            "worker thread");
        return true;
    }
}

bool AppContext::timeout_update_cards() {
    if (m_session_flags[Status::BUSY]) {
        const size_t size = m_cards.size();

        if (m_cards.empty()) {
            return true;
        }

        if (m_next_card_i > size - 1) {
            m_next_card_i = 0;
        }

        auto card = m_cards[m_next_card_i];

        card->update_due_date_label();
        card->set_tooltip_markup(card->create_details_text());

        spdlog::get("app")->debug(
            "[AppContext.timeout_update_cards] CardWidget \"{}\"'s UI has been "
            "updated",
            card->get_card()->get_name());

        m_next_card_i++;

        return true;
    }

    // We stop running at this point
    spdlog::get("app")->debug(
        "[AppContext.timeout_update_cards] Stop card update task");
    return false;
}

void AppContext::register_cardlist_container_update(
    ui::CardlistWidget *cardlist) {
    cardlist->signal_card_added().connect(sigc::track_obj(
        [this](ui::CardWidget *card) {
            this->m_cards.push_back(card);
            card->signal_destroy().connect(
                [this, card]() { std::erase(this->m_cards, card); });
        },
        *cardlist));
    cardlist->signal_card_removed().connect(sigc::track_obj(
        [this](ui::CardWidget *card) { std::erase(this->m_cards, card); },
        *cardlist));
}