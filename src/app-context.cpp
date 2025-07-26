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
        sigc::mem_fun(*this, &AppContext::on_board_loaded));
    m_save_board_dispatcher.connect(
        sigc::mem_fun(*this, &AppContext::on_board_saved));

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

void AppContext::open_board(const std::string &filename) {
    spdlog::get("app")->debug(
        "[AppContext] Dispatch board loading helper thread");
    m_board_load_thread = new std::thread{[this, filename]() {
        try {
            this->m_current_board = this->m_manager.local_open(filename);
        } catch (std::invalid_argument &err) {
            this->m_current_board = nullptr;
        }
        // Calls on_board_loaded
        m_load_board_dispatcher.emit();
    }};
}

void AppContext::close_board() {
    spdlog::get("app")->info(
        "[AppContext] \"{}\" Kanban Board session finished",
        m_current_board->get_name());

    if (m_current_board) {
        if (m_board_save_thread) {
            spdlog::get("app")->debug(
                "[AppContext] Saver worker thread still running. Joining");
            m_board_save_thread->join();
        } else {
            m_manager.local_save(m_current_board);
        }

        // TODO: Extract these cleanup operations onto a helper function
        m_next_card_i = 0;
        m_cards.clear();
        m_timeout_save_task.disconnect();
        m_manager.local_close(m_current_board);
        m_board_widget.clear();
        m_current_board = nullptr;

        m_session_flags[States::LOADING] = false;
        m_session_flags[States::CLEARING] = true;

        if (!m_board_widget.empty()) {
            Glib::signal_idle().connect(
                sigc::mem_fun(*this, &AppContext::idle_clear_board_widget_task),
                Glib::PRIORITY_LOW);
        }
    }
    spdlog::get("app")->info(
        "[AppContext] Kanban board session has been finished");
}

void AppContext::on_board_loaded() {
    if (m_current_board) {
        m_app_window.on_board_view();

        m_session_flags[States::LOADING] = true;

        m_app_window.set_title(m_current_board->get_name());
        m_board_widget.set(m_current_board);
        m_cardlist_i = 0;

        Glib::signal_idle().connect(
            sigc::mem_fun(*this, &AppContext::idle_load_board_widget_task),
            Glib::PRIORITY_LOW);

        // Cleans out the allocated thread object
        m_board_load_thread->join();
        delete m_board_load_thread;
        m_board_load_thread = nullptr;

        if (!m_timeout_save_task.empty()) {
            // The timeout task may not have had the opportunity to exit on its
            // own. Close it before scheduling a new timeout task
            m_timeout_save_task.disconnect();
            spdlog::get("app")->debug(
                "[AppContext] Existing timeout task has been disconnected");
        }

        // Schedule a worker thread saving the current board every SAVE_INTERVAL
        m_timeout_save_task = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_save_board_task),
            AppContext::SAVE_INTERVAL);
    } else {
        // Board has failed to load. Go back to previous state

        spdlog::get("app")->info(
            "[AppContext] Failed to load board. Exiting to main Menu");

        m_session_flags[States::LOADING] = false;
        m_session_flags[States::BUSY] = false;

        Gtk::AlertDialog::create(_("It was not possible to load this board"))
            ->show(m_app_window);
        m_app_window.on_main_menu();
    }
}

void AppContext::on_board_saved() {
    if (m_board_save_thread->joinable()) {
        // A prior call to join has been made because the user has quit
        // the board and save worker thread was still running
        m_board_save_thread->join();
        spdlog::get("app")->debug("[AppContext] Joining save worker thread");
    }

    delete m_board_save_thread;
    m_board_save_thread = nullptr;

    spdlog::get("app")->debug(
        "[AppContext] Save worker thread has been cleaned out");
}

bool AppContext::idle_load_board_widget_task() {
    if (!m_current_board) {
        spdlog::get("app")->warn(
            "[AppContext] Loading operation has been canceled because of "
            "unavailable allocated Board object");
        return false;
    }

    if (m_session_flags[States::CLEARING]) {
        spdlog::get("app")->warn(
            "[AppContext] Current session (\"{}\") attempted to load new "
            "widgets, but remainings from a previous session was not cleared "
            "yet",
            m_current_board->get_name());
        return true;
    }

    if (!m_session_flags[States::LOADING]) {
        spdlog::get("app")->debug("[AppContext] Stopping loading.");
        spdlog::get("app")->debug("[AppContext] Items in m_cards: {}",
                                  m_cards.size());
        return false;
    }

    const auto &data = m_current_board->container().get_data();

    if (m_cardlist_i > (data.size() - 1) || data.empty()) {
        m_session_flags[States::LOADING] = false;
        return false;
    }

    auto cardlist_widget =
        m_board_widget.__add_cardlist(data[m_cardlist_i], false);
    for (const auto &card : cardlist_widget->cardlist()->container()) {
        auto card_widget = cardlist_widget->__add(card);
        m_cards.push_back(card_widget);
    }
    m_cardlist_i++;

    m_timeout_cards_update_task = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &AppContext::timeout_update_cards_task),
        AppContext::UPDATE_INTERVAL);

    m_session_flags[States::LOADING] = true;
    m_session_flags[States::BUSY] = true;
    return true;
}

bool AppContext::idle_clear_board_widget_task() {
    auto cardlist_widget = m_board_widget.pop();
    if (!cardlist_widget) {
        m_session_flags[States::CLEARING] = false;
        m_session_flags[States::BUSY] = false;

        spdlog::get("app")->debug("[AppContext] Board Widget has been cleared");
        spdlog::get("app")->debug("[AppContext] m_cards after clearing: {}",
                                  m_cards.size());
        return false;
    }

    m_session_flags[States::BUSY] = true;
    return true;
}

// FIXME: This procedure is executed even when m_current board is not available
bool AppContext::timeout_save_board_task() {
    if (!m_current_board) {
        // There is no board to save, stop this timeout procedure
        spdlog::get("app")->warn(
            "[AppContext] Attempted to save board, however it is now "
            "unavailable. Stop timeout save procedure");
        return false;
    } else if (m_current_board->modified()) {
        if (m_board_save_thread) {
            // It simply means that there is still a worker thread
            // active, so pass it
            spdlog::get("app")->debug(
                "[AppContext] Tried to schedule saver worker"
                "thread however there was one still running");
            return true;
        } else {
            spdlog::get("app")->debug(
                "[AppContext] saver thread has been scheduled");
            m_board_save_thread = new std::thread{[this]() {
                m_manager.local_save(m_current_board);
                m_save_board_dispatcher.emit();  // calls on_board_saved()
            }};
            return true;
        }
    } else {
        spdlog::get("app")->debug(
            "[AppContext] No modifications registered. Don't scheduled save "
            "thread");
        return true;
    }
}

bool AppContext::timeout_update_cards_task() {
    if (m_session_flags[States::BUSY]) {
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
        card->set_tooltip_text(card->create_details_text());

        spdlog::get("app")->debug(
            "[AppContext] CardWidget \"{}\" has been updated",
            card->get_card()->get_name());

        m_next_card_i++;

        return true;
    }

    // We stop running at this point
    m_timeout_cards_update_task.disconnect();
    return false;
}
