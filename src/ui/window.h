#pragma once

#include <gtkmm.h>

#include "board-widget.h"
#include "create_board_dialog.h"
#include "preferences-board-dialog.h"
#include "board-card-button.h"
#include "window-controller.h"

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
    DeleteBoardsBar(WindowController& window_controller);

private:
    Gtk::Box root;
    Gtk::Label bar_text;
    Gtk::Button bar_button_delete, bar_button_cancel;

    WindowController& window_controller;
};

/**
 * Progress application window.
 */
class ProgressWindow : public Gtk::Window {
public:
    ProgressWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    ~ProgressWindow() override;

    WindowController& get_window_controller();

private:
    const Glib::RefPtr<Gtk::Builder> window_builder;
    ui::ProgressAboutDialog about_dialog;
    ui::CreateBoardDialog* create_board_dialog;
    ui::PreferencesBoardDialog* preferences_board_dialog;
    ui::DeleteBoardsBar delete_boards_bar;
    ui::BoardWidget board_widget;

    WindowController window_controller;

    void setup_menu_button();
};
}  // namespace ui