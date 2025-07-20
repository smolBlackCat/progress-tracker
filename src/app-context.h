#pragma once

#include <widgets/board-widget.h>
#include <widgets/card-widget.h>
#include <widgets/cardlist-widget.h>

#include <thread>

namespace ui {
class ProgressWindow;
}

enum class States {
    CLEARING, LOADING, BUSY
};

/**
 * @brief Application controller class
 *
 * AppContext key function is to implement the communication interface
 * between the app core and the app widgets by delegating the functionality
 * logic into this class. AppContext is responsible to control the usual
 * application assumptions (e.g. current saving operations still running) and
 * properly communicate it to the app components (e.g widgets, core).
 */
class AppContext {
public:
    AppContext(ui::ProgressWindow &app_window, ui::BoardWidget &board_widget,
               BoardManager &manager);

    /**
     * @brief Starts a kanban board session
     *
     * @param filename file path where Progress board is located
     */
    void open_board(const std::string &filename);

    /**
     * @brief Ends a kanban board session
     */
    void close_board();

protected:
    /**
     * @brief Emitted by m_load_board_dispatcher. Begin scheduling and
     * transitioning the app view to a Kanban board session
     */
    void on_board_loaded();

    /**
     * @brief Emitted by m_save_board_dispatcher. It cleans the thread to
     * indicate a new worker thread may be scheduled
     */
    void on_board_saved();

    bool idle_load_board_widget_task();
    bool idle_clear_board_widget_task();
    bool timeout_save_board_task();

    ui::ProgressWindow &m_app_window;
    ui::BoardWidget &m_board_widget;

    // Board loading and saving
    BoardManager &m_manager;
    std::thread *m_board_save_thread, *m_board_load_thread;
    Glib::Dispatcher m_load_board_dispatcher, m_save_board_dispatcher;
    sigc::connection m_timeout_save_task;

    // BoardWidget context
    std::shared_ptr<Board> m_current_board;
    std::unordered_map<States, bool> m_board_widget_flags = {
        {States::LOADING, false},
        {States::CLEARING, false},
        {States::BUSY, false},
    };
    std::vector<ui::CardWidget *> m_cards;
    ssize_t m_cardlist_index = 0;
};
