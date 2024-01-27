#pragma once

#include <gtkmm.h>

#include "../core/board.h"
#include "board-widget.h"
#include "create_board_dialog.h"

namespace ui {
/**
 * Progress app about dialog.
 */
class ProgressAboutDialog : public Gtk::AboutDialog {
public:
    ProgressAboutDialog(Gtk::Window& parent);
    ~ProgressAboutDialog() override;
};

class ProgressWindow;

/**
 * @brief Bar widget that asks user confirmation to delete selected boards.
*/
class DeleteBoardsBar : public Gtk::Revealer {
public:
    DeleteBoardsBar(Gtk::FlowBox& boards_grid, ui::ProgressWindow& app_window);

private:
    Gtk::Box root;
    Gtk::Label bar_text;
    Gtk::Button bar_button_delete, bar_button_cancel;

    Gtk::FlowBox& boards_grid;
    ui::ProgressWindow& app_window;

    void on_delete_boards();
};

/**
 * Progress application window.
 */
class ProgressWindow : public Gtk::Window {
public:
    ProgressWindow();
    ~ProgressWindow() override;

    void add_board(std::string board_filepath);
    void off_delete_board();

private:
    Gtk::HeaderBar header_bar;
    Gtk::Button add_board_button, home_button;
    Glib::RefPtr<Gio::Menu> board_grid_menu, board_main_menu;
    Gtk::MenuButton menu_button, board_menu_button;
    Gtk::Box root;
    Gtk::FlowBox board_grid;
    ui::BoardWidget board_widget;
    ui::DeleteBoardsBar delete_boards_bar;
    Gtk::Stack stack;

    /**
     * There are two kinds of pages:
     *
     * board_grid: Shows a view of various boards to be worked on by the user
     *
     * board_main: Shows the kanban todo list
     */
    std::string current_page;
    int index = 0;
    bool on_delete_mode = false;

    ui::ProgressAboutDialog about_dialog;
    ui::CreateBoardDialog* create_board_dialog;

    void setup_menu_button();
    void show_about();
    void show_create_board();
    void go_to_main_board();
    void go_to_main_menu();
    void on_delete_board();
};
}  // namespace ui