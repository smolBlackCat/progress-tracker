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
 * Progress app about dialog.
 */
class ProgressAboutDialog {
public:
    ProgressAboutDialog(Gtk::Window& parent);
    ~ProgressAboutDialog();

    /**
     * @brief Presents the about dialog on screen
     */
    void show();

protected:
    void setup();

    AdwDialog* about_dialogp;
    Gtk::Widget& parent;
};

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

    ui::ProgressWindow& app_window;
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

    ProgressWindow(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& b);
    ~ProgressWindow() override;

    void add_board(const std::string& board_filepath);

    /**
     * @brief Presents dialog for creating boards
     */
    void show_create_board_dialog();

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

    bool on_delete_mode = false;

    ui::ProgressAboutDialog about_dialog;
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
    bool on_window_close();
};
}  // namespace ui

