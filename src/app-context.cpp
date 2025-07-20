#include "app-context.h"

#include <glibmm/i18n.h>
#include <window.h>

AppContext::AppContext(ui::ProgressWindow &app_window,
                       ui::BoardWidget &board_widget, BoardManager &manager)
    : m_app_window(app_window),
      m_board_widget(board_widget),
      m_manager(manager) {
    m_load_board_dispatcher.connect(
        sigc::mem_fun(*this, &AppContext::on_board_loaded));
    m_save_board_dispatcher.connect(
        sigc::mem_fun(*this, &AppContext::on_board_saved));
}

void AppContext::open_board(const std::string &filename) {
    spdlog::get("app")->debug(
        "[AppContext] Dispatch board loading helper thread");
    m_board_load_thread = new std::thread{[this, filename]() {
        try {
            this->m_current_board = this->m_manager.local_open(filename);
        } catch (std::invalid_argument &err) {
            this->m_current_board = nullptr;
            spdlog::get("app")->error("[AppContext] Failed to load board: {}",
                                      err.what());
        }

        // Calls on_board_loaded
        m_load_board_dispatcher.emit();
    }};
}

void AppContext::close_board() {
    spdlog::get("app")->info("[AppContext] Finish current board session");
    m_board_widget_flags["loading"] = false;

    if (m_current_board) {
        if (m_board_save_thread) {
            spdlog::get("app")->debug(
                "[AppContext] saver worker thread is still running. Wait "
                "for it to finish");
            m_board_save_thread->join();
        } else {
            m_manager.local_save(m_current_board);
        }
        m_manager.local_close(m_current_board);

        m_board_widget.clear();

        m_board_widget_flags["clearing"] = true;
        if (!m_board_widget.empty()) {
            Glib::signal_idle().connect(
                sigc::mem_fun(*this, &AppContext::idle_clear_board_widget_task),
                Glib::PRIORITY_LOW);
        }
        m_current_board = nullptr;
    }
    spdlog::get("app")->info(
        "[AppContext] Kanban board session has been finished");
}

void AppContext::on_board_loaded() {
    if (m_current_board) {
        m_app_window.on_board_view();

        m_board_widget_flags["loading"] = true;

        m_app_window.set_title(m_current_board->get_name());
        m_board_widget.set(m_current_board);
        m_cardlist_index = 0;

        Glib::signal_idle().connect(
            sigc::mem_fun(*this, &AppContext::idle_load_board_widget_task),
            Glib::PRIORITY_LOW);

        // Cleans out the allocated thread object
        m_board_load_thread->join();
        delete m_board_load_thread;
        m_board_load_thread = nullptr;

        // Schedule a worker thread saving the current board every SAVE_INTERVAL
        if (!m_timeout_save_task.empty()) {
            // The timeout task may not have had the opportunity to exit on its
            // own. Close it before scheduling a new timeout task
            m_timeout_save_task.disconnect();
            spdlog::get("app")->debug(
                "[AppContext] Existing timeout task has been disconnected");
        }

        m_timeout_save_task = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_save_board_task),
            ui::BoardWidget::SAVE_INTERVAL);
    } else {
        // Because loading the current board has failed, let's reset the context
        m_board_widget_flags["loading"] = false;
        m_board_widget_flags["busy"] = false;

        Gtk::AlertDialog::create(_("It was not possible to load this board"))
            ->show(m_app_window);
        m_app_window.on_main_menu();

        spdlog::get("app")->error(
            "[AppContext] BoardWidget loading has failed");
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
        return false;
    }

    if (m_board_widget_flags["clearing"]) {
        spdlog::get("app")->debug(
            "[AppContext] BoardWidget is still clearing. Waiting for it to be "
            "available");
        return true;
    }

    if (!m_board_widget_flags["loading"]) {
        spdlog::get("app")->debug("[AppContext] Stopping loading.");
        return false;
    }

    const auto data = m_current_board->container().get_data();

    if (m_cardlist_index > (data.size() - 1) || !m_current_board ||
        data.empty()) {
        m_board_widget_flags["loading"] = false;
        return false;
    }

    auto m = m_board_widget.__add_cardlist(data[m_cardlist_index], false);
    m_cardlist_index++;

    m_board_widget_flags["loading"] = true;
    return true;
}

bool AppContext::idle_clear_board_widget_task() {
    if (!m_board_widget_flags["clearing"]) {
        m_board_widget_flags["busy"] = false;
        spdlog::get("app")->debug("[AppContext] Board widget has been cleared");
        return false;
    } else {
        m_board_widget_flags["clearing"] = true;
        m_board_widget_flags["busy"] = true;
        auto cardlist_widget = m_board_widget.pop();
        if (!cardlist_widget) m_board_widget_flags["clearing"] = false;
        return true;
    }
}

bool AppContext::timeout_save_board_task() {
    if (!m_current_board) {
        // There is no board to save, stop this timeout procedure
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
