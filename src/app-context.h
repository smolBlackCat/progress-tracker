#pragma once

#include <core/item.h>
#include <glib.h>
#include <widgets/board-widget.h>
#include <widgets/card-widget.h>
#include <widgets/cardlist-widget.h>

#include <thread>
#include <unordered_map>
#include <vector>

#include "core/cardlist.h"
#include "gtkmm/version.h"
#include "widgets/task-widget.h"

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

    static ui::CardWidget* builder_card_widget(
        const std::shared_ptr<Card>& card);

    AppContext(ui::ProgressWindow& app_window, ui::BoardWidget& board_widget,
               BoardManager& manager);

    /**
     * @brief Starts a kanban board session
     *
     * @param filename file path where Progress board is located
     */
    void open_session(const std::string& filename);

    /**
     * @brief Ends a kanban board session
     */
    void close_session();

protected:
    /**
     * @brief Resets to context's variables to an initial state
     */
    void reset_session_state();

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

    /**
     * @brief Toggles the timeout UI update procedure.
     *
     * @details Whenever the window is suspended, there is no need for further
     * UI updates since they won't be visible to the user. The proper solution
     * is to disconnect the procedure from the timeout signal whenever the
     * window is minimised, and reconnect it when the window is maximised again
     */
    void toggle_timeout_update_cards();

    // Binding methods

    void bind(const std::shared_ptr<Board>& db_board, ui::BoardWidget* board_w);
    void bind(const std::shared_ptr<CardList>& db_cardlist,
              ui::CardlistWidget* cardlist_w);
    void bind(const std::shared_ptr<Card>& db_card, ui::CardWidget* card_w);
    void bind(const std::shared_ptr<Task>& db_task, ui::TaskWidget* task_w);

    void clear_binds();

    /**
     * @brief Pushs a card widget object pointer to the updating system queue.
     * Also, the timeout update procedure is scheduled if it has been
     * deactivated
     * 
     * @param card_w card widget object pointer
     */
    void card_update_queue_push(ui::CardWidget* card_w);

    /**
     * @brief Save the current session if there is one
     * */
    bool on_window_closed();

    bool idle_load_session();
    bool idle_clear_session();
    bool timeout_save_session();
    bool timeout_update_cards();

    ui::ProgressWindow& m_app_window;
    ui::BoardWidget& m_board_widget;

    // Session Context
    std::shared_ptr<Board> m_current_board;
    std::unordered_map<Status, bool> m_session_flags = {
        {Status::LOADING, false},
        {Status::CLEARING, false},
        {Status::BUSY, false},
    };
    BoardManager& m_manager;
    std::thread m_board_save_thread, m_board_load_thread;
    Glib::Dispatcher m_load_board_dispatcher, m_save_board_dispatcher;
    sigc::connection m_timeout_save_cnn, m_timeout_cards_update_cnn,
        m_idle_load_session_cnn;

#if GTKMM_CHECK_VERSION(4, 12, 0)
    sigc::connection m_suspended_tracker_cnn;
#else
    long m_suspended_tracker_handler_id = 0;
#endif

    // BoardWidget Context
    std::vector<sigc::scoped_connection> m_board_widget_cnns, m_cardlists_cnns,
        m_cards_cnns, m_tasks_cnns, m_card_dialog_cnns;

    std::unordered_map<ui::CardlistWidget*, std::shared_ptr<CardList>>
        m_bound_cardlists;
    std::unordered_map<ui::CardWidget*, std::shared_ptr<Card>> m_bound_cards;
    std::unordered_map<ui::TaskWidget*, std::shared_ptr<Task>> m_bound_tasks;
    std::vector<ui::CardWidget*> m_cards;
    ssize_t m_next_card_i = 0;
    ssize_t m_cardlist_i = 0;

private:
    void setup_board_widget();
    void register_cardlist_container_update(ui::CardlistWidget* cardlist);
};
