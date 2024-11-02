#pragma once

#include <adwaita.h>
#include <gtkmm.h>
#include <widgets/board-widget.h>

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
    DeleteBoardsBar(ui::ProgressWindow& app_window);

private:
    Gtk::Box root;
    Gtk::Label bar_text;
    Gtk::Button bar_button_delete, bar_button_cancel;
};

/**
 * Progress application window.
 */
class ProgressWindow : public Gtk::ApplicationWindow {
public:
    static constexpr const char* STYLE_DARK_CSS =
        "/io/github/smolblackcat/Progress/style-dark.css";
    static constexpr const char* STYLE_CSS =
        "/io/github/smolblackcat/Progress/style.css";
    static constexpr const char* CREATE_BOARD_DIALOG =
        "/io/github/smolblackcat/Progress/create-board-dialog.ui";

    ProgressWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& b,
                   Glib::RefPtr<Gio::Settings>& progress_settings);
    ~ProgressWindow() override;

    /**
     * @brief Adds local boards from a given backend
     */
    void add_local_board(BoardBackend board_backend);

    /**
     * @brief Enters deletion mode, where the user will select all boards to
     * be deleted
     */
    void on_delete_board_mode();

    /**
     * @brief Exits deletion mode
     */
    void off_delete_board_mode();

    /**
     * @brief Changes the application view to the board grid view
     */
    void on_main_menu();

    /**
     * @brief Changes the application view to board view
     */
    void on_board_view();

    /**
     * @brief Deletes all selected boards if window is in delete mode.
     */
    void delete_selected_boards();

protected:
    AdwStyleManager* adw_style_manager;
    Glib::RefPtr<Gtk::CssProvider> css_provider;
    Glib::RefPtr<Gio::Settings>& progress_settings;

    bool on_delete_mode = false;

    ui::CreateBoardDialog* create_board_dialog;
    ui::PreferencesBoardDialog* preferences_board_dialog;
    ui::DeleteBoardsBar delete_boards_bar;
    ui::BoardWidget board_widget;
    Gtk::Button *home_button_p, *add_board_button_p;
    Gtk::Overlay* app_overlay_p;
    Gtk::Stack* app_stack_p;
    Gtk::FlowBox* boards_grid_p;
    Glib::RefPtr<Gio::MenuModel> board_grid_menu_p, board_menu_p;
    Gtk::MenuButton* app_menu_button_p;

    void setup_menu_button();
    void load_appropriate_style();
    bool on_close_request() override;
};
}  // namespace ui
