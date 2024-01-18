#pragma once

#include <gtkmm/aboutdialog.h>
#include <gtkmm/button.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/label.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/window.h>

#include "../core/board.h"
#include "create_board_dialog.h"
#include "board-widget.h"

namespace ui {
/**
 * Progress app about dialog.
 */
class ProgressAboutDialog : public Gtk::AboutDialog {
public:
    ProgressAboutDialog(Gtk::Window& parent);
    ~ProgressAboutDialog() override;
};

/**
 * Progress application window.
 */
class ProgressWindow : public Gtk::Window {
public:
    ProgressWindow();
    ~ProgressWindow() override;

    void add_board(Board* board);

private:
    Gtk::HeaderBar header_bar;
    Gtk::Button add_board_button, home_button;
    Gtk::MenuButton menu_button, board_menu_button;
    Gtk::FlowBox board_grid;
    ui::BoardWidget board_widget;
    Gtk::Stack stack;

    /**
     * There are two kinds of pages:
     *
     * board_grid: Shows a view of various boards to be worked on by the user
     *
     * board_main: Shows the kanban todo list
     */
    std::string current_page;

    ui::ProgressAboutDialog about_dialog;
    ui::CreateBoardDialog* create_board_dialog;

    void setup_menu_button();
    void show_about();
    void show_create_board();
    void go_to_main_board();
    void go_to_main_menu();
};
}  // namespace ui