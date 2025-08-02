#pragma once

#include <widgets/board-widget.h>
#include <widgets/card-widget.h>
#include <widgets/cardlist-widget.h>

#include <thread>

namespace ui {
class ProgressWindow;
}

enum class Status { CLEARING, LOADING, BUSY };

/**
 * @brief Application controller class
 *
 * AppContext implements a communication interface between the app's widgets and
 * the app's core. AppContext is responsible for controlling sessions started or
 * finished by the user and translating this to the programme's components.
 *
 * Threading considerations:
 *
 * Progress utilises threads to not block the UI when loading or saving
 * Progress boards into the disk. These threads, when scheduled to run, always
 * notify the context handler's dispatchers to reflect any changes made by those
 * threads into the application, ensuring proper synchronisation and resource
 * deallocation.
 *
 * By default, threads handling loading and saving must be inactive after
 * closing a Kanban board session.
 */
class AppContext {
public:
    static constexpr int SAVE_INTERVAL = 1000 * 10;
    static constexpr int UPDATE_INTERVAL = 1000 * 3;

    AppContext(ui::ProgressWindow &app_window, ui::BoardWidget &board_widget,
               BoardManager &manager);

    /**
     * @brief Starts a kanban board session
     *
     * @param filename file path where Progress board is located
     */
    void open_session(const std::string &filename);

    /**
     * @brief Ends a kanban board session
     */
    void close_session();

protected:
    /**
     * @brief Resets to context's variables to an initial state
     */
    void reset_state();

    /**
     * @brief Schedule auto-save, widgets updating, and lazy loading tasks to
     * start a Kanban session
     */
    void on_session_loaded();

    /**
     * @brief Callback to execute whenever the saver worker thread has finished
     * its execution
     */
    void on_session_saved();

    bool idle_load_session();
    bool idle_clear_session();
    bool timeout_save_session();
    bool timeout_update_cards();

    ui::ProgressWindow &m_app_window;
    ui::BoardWidget &m_board_widget;

    // Board loading and saving
    BoardManager &m_manager;
    std::thread m_board_save_thread, m_board_load_thread;
    Glib::Dispatcher m_load_board_dispatcher, m_save_board_dispatcher;
    sigc::connection m_timeout_save_cnn, m_timeout_cards_update_cnn,
        m_idle_load_session_cnn;

    // BoardWidget context
    std::shared_ptr<Board> m_current_board;
    std::unordered_map<Status, bool> m_session_flags = {
        {Status::LOADING, false},
        {Status::CLEARING, false},
        {Status::BUSY, false},
    };
    std::vector<ui::CardWidget *> m_cards;
    ssize_t m_next_card_i = 0;
    ssize_t m_cardlist_i = 0;
};
