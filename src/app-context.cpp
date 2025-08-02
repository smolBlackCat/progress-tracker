#include "app-context.h"

#include <glibmm/i18n.h>
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

    // FIXME: This is too much to read. This asks the cardlist added to board
    // widget to sign whether a card widget has been added to it. In addition,
    // it asks the added card to sign whether it has been destructed.
    m_board_widget.signal_cardlist_added().connect(
        [this](ui::CardlistWidget *cardlist) {
            cardlist->signal_card_added().connect(sigc::track_obj(
                [this](ui::CardWidget *card) {
                    this->m_cards.push_back(card);
                    card->signal_destroy().connect(
                        [this, card]() { std::erase(this->m_cards, card); });
                },
                *cardlist));

            cardlist->signal_card_removed().connect(sigc::track_obj(
                [this](ui::CardWidget *card) {
                    std::erase(this->m_cards, card);
                },
                *cardlist));
        });
}

void AppContext::open_session(const std::string &filename) {
    spdlog::get("app")->debug(
        "[AppContext] Dispatch board loading helper thread");
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
    spdlog::get("app")->info(
        "[AppContext] \"{}\" Kanban Board session finished",
        m_current_board->get_name());

    if (m_current_board) {
        if (m_board_save_thread.joinable()) {
            spdlog::get("app")->debug(
                "[AppContext] Saver worker thread still running. Joining");
            m_board_save_thread.join();
        } else {
            m_manager.local_save(m_current_board);
        }
        m_manager.local_close(m_current_board);

        reset_state();

        m_session_flags[Status::LOADING] = false;
        m_session_flags[Status::CLEARING] = true;

        if (!m_board_widget.empty()) {
            Glib::signal_idle().connect(
                sigc::mem_fun(*this, &AppContext::idle_clear_session),
                Glib::PRIORITY_LOW);
        }
    }
}

void AppContext::reset_state() {
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
    // Cleans out the allocated thread object
    if (m_board_load_thread.joinable()) {
        m_board_load_thread.join();
    }

    if (m_current_board) {
        m_app_window.on_board_view();

        m_session_flags[Status::LOADING] = true;

        m_app_window.set_title(m_current_board->get_name());
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
        spdlog::get("app")->info(
            "[AppContext] Failed to load board. Exiting to main Menu");

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
        spdlog::get("app")->debug("[AppContext] Joining save worker thread");
    }

    spdlog::get("app")->debug(
        "[AppContext] Save worker thread has been cleaned out");
}

bool AppContext::idle_load_session() {
    if (!m_current_board) {
        spdlog::get("app")->warn(
            "[AppContext] Current board is not valid. Stopping loading task");
        m_session_flags[Status::LOADING] = false;
        return false;
    }

    if (m_session_flags[Status::CLEARING]) {
        spdlog::get("app")->warn(
            "[AppContext] Current session (\"{}\") has to wait for the Board "
            "to be empty",
            m_current_board->get_name());
        return true;
    }

    if (!m_session_flags[Status::LOADING]) {
        spdlog::get("app")->debug("[AppContext] Stopping loading.");
        spdlog::get("app")->debug("[AppContext] Items in m_cards: {}",
                                  m_cards.size());
        return false;
    }

    const auto &data = m_current_board->container().get_data();
    if (m_cardlist_i > (data.size() - 1) || data.empty()) {
        m_session_flags[Status::LOADING] = false;
        return false;
    }

    ui::CardlistWidget *cardlist_widget = Gtk::make_managed<ui::CardlistWidget>(
        m_board_widget, data[m_cardlist_i], false);
    m_board_widget.append(*cardlist_widget);

    for (const auto &card : cardlist_widget->cardlist()->container()) {
        ui::CardWidget *card_widget =
            Gtk::make_managed<ui::CardWidget>(card, false);
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

        spdlog::get("app")->debug("[AppContext] Board Widget has been cleared");
        spdlog::get("app")->debug("[AppContext] m_cards after clearing: {}",
                                  m_cards.size());
        return false;
    }

    m_session_flags[Status::BUSY] = true;
    return true;
}

bool AppContext::timeout_save_session() {
    if (!m_current_board) {
        // There is no board to save, stop this timeout procedure
        spdlog::get("app")->warn(
            "[AppContext] Attempted to save board, however it is now "
            "unavailable. Stop timeout save procedure");
        return false;
    } else if (m_current_board->modified()) {
        if (m_board_save_thread.joinable()) {
            spdlog::get("app")->debug(
                "[AppContext] Attempted to schedule saver worker"
                "thread, however, it has not been joined");
            return true;
        } else {
            m_board_save_thread = std::thread{[this]() {
                m_manager.local_save(m_current_board);
                m_save_board_dispatcher.emit();
            }};
            spdlog::get("app")->debug(
                "[AppContext] saver worker thread has been scheduled");
            return true;
        }
    } else {
        spdlog::get("app")->debug(
            "[AppContext] No modifications registered. Don't schedule a saver "
            "worker thread");
        return true;
    }
}

bool AppContext::timeout_update_cards() {
    if (m_session_flags[Status::BUSY]) {
        const ssize_t size = m_cards.size();

        if (m_cards.empty()) {
            return true;
        }

        if (m_next_card_i > size - 1) {
            spdlog::get("app")->debug(
                "[AppContext] Restarting m_next_card_i to zero");
            m_next_card_i = 0;
        }

        auto card = m_cards[m_next_card_i];

        card->update_due_date_label();
        card->set_tooltip_markup(card->create_details_text());

        spdlog::get("app")->debug(
            "[AppContext] CardWidget \"{}\" has been updated",
            card->get_card()->get_name());

        m_next_card_i++;

        return true;
    }

    // We stop running at this point
    spdlog::get("app")->debug(
        "[App Context] Stopping timeout card updates task");
    return false;
}
