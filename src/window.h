#pragma once

#include <adwaita.h>
#include <gtkmm.h>
#include <widgets/board-widget.h>

#include "dialog/card-dialog.h"

namespace ui {

class BoardWidget;
class CreateBoardDialog;
class PreferencesBoardDialog;
class ProgressWindow;

/**
 * @brief Bar widget that asks user confirmation to delete selected boards.
 */
class DeleteBoardsBar : public Gtk::Revealer {
public:
    /**
     * @brief Constructs a DeleteBoardsBar object.
     *
     * @param app_window Reference to the ProgressWindow.
     */
    DeleteBoardsBar(ui::ProgressWindow& app_window);

private:
    Gtk::Box root;        ///< Root container for the delete boards bar.
    Gtk::Label bar_text;  ///< Label displaying the delete confirmation text.
    Gtk::Button bar_button_delete,
        bar_button_cancel;  ///< Buttons for delete and cancel actions.
};

/**
 * @brief Progress application window.
 *
 * The ProgressWindow class represents the main application window. It provides
 * functionalities for managing boards, switching views, and handling user
 * interactions.
 */
class ProgressWindow : public Gtk::ApplicationWindow {
public:
    static constexpr const char* STYLE_DARK_CSS =
        "/io/github/smolblackcat/Progress/style-dark.css";  ///< Path to the
                                                            ///< dark style CSS
                                                            ///< file.
    static constexpr const char* STYLE_CSS =
        "/io/github/smolblackcat/Progress/style.css";  ///< Path to the default
                                                       ///< style CSS file.
    static constexpr const char* CREATE_BOARD_DIALOG =
        "/io/github/smolblackcat/Progress/create-board-dialog.ui";  ///< Path to
                                                                    ///< the
                                                                    ///< create
                                                                    ///< board
                                                                    ///< dialog
                                                                    ///< UI
                                                                    ///< file.

    /**
     * @brief Constructs a ProgressWindow object.
     *
     * @param cobject Pointer to the base object type.
     * @param b Reference to the Gtk::Builder.
     * @param progress_settings Reference to the application settings.
     */
    ProgressWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& b,
                   Glib::RefPtr<Gio::Settings>& progress_settings);

    /**
     * @brief Destructor.
     */
    ~ProgressWindow() override;

    /**
     * @brief Adds local boards from a given backend.
     *
     * @param board_backend The backend from which to load the boards.
     */
    void add_local_board(BoardBackend board_backend);

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
    AdwStyleManager* adw_style_manager;  ///< Style manager for Adwaita.
    Glib::RefPtr<Gtk::CssProvider> css_provider;  ///< CSS provider for styling.
    Glib::RefPtr<Gio::Settings>&
        progress_settings;  ///< Reference to the application settings.

    std::shared_ptr<Board> cur_board;  ///< Pointer to the current board
                                       ///< (nullptr when not in board-page).
    BoardCardButton* cur_board_entry;  ///< Pointer to the current board entry
                                       ///< (nullptr when not in board-page).
    Glib::Dispatcher
        dispatcher;  ///< Dispatcher for handling asynchronous events.

    bool on_delete_mode =
        false;  ///< Flag indicating if the application is in delete mode.
    ui::DeleteBoardsBar delete_boards_bar;  ///< Delete boards bar widget.
    ui::BoardWidget board_widget;           ///< Board widget.
    Gtk::ShortcutsWindow* sh_window;        ///< Shortcuts window.
    Gtk::Button *home_button_p,
        *add_board_button_p;      ///< Buttons for home and add board actions.
    Gtk::Overlay* app_overlay_p;  ///< Overlay for the application.
    Gtk::Stack* app_stack_p;      ///< Stack for switching between views.
    Gtk::FlowBox* boards_grid_p;  ///< Flow box for displaying boards.
    Glib::RefPtr<Gio::MenuModel> board_grid_menu_p,
        board_menu_p;  ///< Menu models for board grid and board menus.
    Gtk::MenuButton*
        app_menu_button_p;  ///< Menu button for the application menu.

    CardDetailsDialog card_dialog;  ///< Card details dialog.

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
     * @brief Handles the close event.
     *
     * @return True if the event was handled, false otherwise.
     */
    bool on_close();
};

}  // namespace ui