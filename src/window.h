#pragma once

#include <adwaita.h>
#include <gtkmm.h>
#include <widgets/board-card-button.h>
#include <widgets/board-widget.h>

#include <thread>

#include "core/board-manager.h"
#include "dialog/card-dialog.h"

namespace ui {

class BoardWidget;
class CreateBoardDialog;
class PreferencesBoardDialog;

/**
 * @brief Main application window
 *
 * It provides functionalities for managing boards, switching views, and
 * handling user interactions.
 */
class ProgressWindow : public Gtk::ApplicationWindow {
public:
    static constexpr const char* STYLE_DARK_CSS =
        "/io/github/smolblackcat/Progress/style-dark.css";

    static constexpr const char* STYLE_CSS =
        "/io/github/smolblackcat/Progress/style.css";

    static constexpr const char* CREATE_BOARD_DIALOG =
        "/io/github/smolblackcat/Progress/create-board-dialog.ui";

    /**
     * @brief Constructs a ProgressWindow object.
     *
     * @param cobject Pointer to the base object type.
     * @param b Reference to the Gtk::Builder.
     * @param progress_settings Reference to the application settings.
     */
    ProgressWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& b,
                   Glib::RefPtr<Gio::Settings>& progress_settings,
                   BoardManager& manager);

    /**
     * @brief Destructor.
     */
    ~ProgressWindow() override;

    void add_board_handler(LocalBoard board_entry);
    void remove_board_handler(LocalBoard board_entry);
    void save_board_handler(LocalBoard board_entry);

    /**
     * @brief Adds local boards from a given backend.
     *
     * @param board_backend The backend from which to load the boards.
     */
    void add_local_board_entry(LocalBoard board_entry);

    /**
     * @brief Idle process task filling board widget
     */
    void load(const std::shared_ptr<Board> board);

    /**
     * @brief Idle process task clearing board widget
     */
    void unload();

    /**
     * @brief Enters deletion mode, where the user will select all boards to be
     * deleted.
     */
    void on_delete_board_mode();

    /**
     * @brief Exits deletion mode.
     */
    void off_delete_board_mode();

    /**
     * @brief Changes the application view to the board grid view.
     */
    void on_main_menu();

    /**
     * @brief Changes the application view to board view.
     */
    void on_board_view();

    /**
     * @brief Deletes all selected boards if window is in delete mode.
     */
    void delete_selected_boards();

    /**
     * @brief Shows the general information about the application.
     */
    void show_about_dialog();

    /**
     * @brief Shows the card details dialog.
     *
     * @param card_widget Pointer to the CardWidget to show details for.
     */
    void show_card_dialog(CardWidget* card_widget);

    /**
     * @brief Shows the shortcuts dialog.
     */
    void show_shortcuts_dialog();

protected:
    BoardManager& m_manager;

    // Context
    std::shared_ptr<Board> cur_board;
    BoardCardButton* cur_board_entry;
    Glib::Dispatcher load_board_dispatcher, save_board_dispatcher;

    sigc::connection timeout_save_task;

    // A nullptr means no worker threads currently running
    std::thread *load_board_thread = nullptr, *save_board_thread = nullptr;

    bool on_delete_mode = false;

    // Whether there is a idle process task adding new items to a board
    bool board_widget_loading = false;

    // Whether there is a idle process task clearing board widget
    bool board_widget_clearing = false;

    // Board Widget is busy when a user board is loaded and being currently used
    bool board_widget_busy = false;

    // We need this value to keep track of the next cardlist to add in the idle
    // process
    ssize_t cardlist_index = 0;
    std::vector<CardlistWidget*> m_cardlists;

    // Settings
    Glib::RefPtr<Gtk::CssProvider> css_provider;
    Glib::RefPtr<Gio::Settings>& progress_settings;

    // Widgets
    AdwStyleManager* adw_style_manager;
    ui::BoardWidget board_widget;
    Gtk::ShortcutsWindow* sh_window;
    Gtk::Button *home_button_p, *add_board_button_p, *board_delete_button,
        *cancel_delete_button;
    Gtk::Overlay* app_overlay_p;
    Gtk::Stack* app_stack_p;
    Gtk::FlowBox* boards_grid_p;
    Gtk::ActionBar* action_bar_p;
    Glib::RefPtr<Gio::MenuModel> board_grid_menu_p, board_menu_p;
    Gtk::MenuButton* app_menu_button_p;

#if not GTKMM_CHECK_VERSION(4, 14, 0)
    std::vector<BoardCardButton*> entry_buttons;
#endif

    CardDetailsDialog card_dialog;

    /**
     * @brief Sets up the menu button.
     */
    void setup_menu_button();

    /**
     * @brief Loads the appropriate style based on the settings.
     */
    void load_appropriate_style();

    /**
     * @brief Called when the loader thread is done working, sets board widget
     * and changes app's view.
     */
    void on_board_loading_done();

    /**
     * @brief Called when the saver thread is done working, resets the current
     * board
     */
    void on_board_saved();

    /**
     * @brief Handles the close event.
     *
     * @return True if the event was handled, false otherwise.
     */
    bool on_close();
};

}  // namespace ui

