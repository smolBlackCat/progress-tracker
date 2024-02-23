#pragma once

#include <gtkmm.h>

#include "../core/board.h"
#include "board-widget.h"
#include "create_board_dialog.h"
#include "preferences-board-dialog.h"
#include "board-card-button.h"

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
    DeleteBoardsBar(Gtk::FlowBox* boards_grid, ui::ProgressWindow& app_window);

private:
    Gtk::Box root;
    Gtk::Label bar_text;
    Gtk::Button bar_button_delete, bar_button_cancel;

    Gtk::FlowBox* boards_grid;
    ui::ProgressWindow& app_window;

    void on_delete_boards();
};

/**
 * Progress application window.
 */
class ProgressWindow : public Gtk::Window {
public:
    ProgressWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    ~ProgressWindow() override;

    void add_board(std::string board_filepath);
    void off_delete_board();

private:
    const Glib::RefPtr<Gtk::Builder> window_builder;

    bool on_delete_mode = false;

    ui::ProgressAboutDialog about_dialog;
    ui::CreateBoardDialog* create_board_dialog;
    ui::PreferencesBoardDialog* preferences_board_dialog;
    ui::DeleteBoardsBar delete_boards_bar;
    ui::BoardWidget board_widget;

    void setup_menu_button();
    void show_about();
    void show_create_board();
    void go_to_main_menu();
    void on_delete_board();
};
}  // namespace ui