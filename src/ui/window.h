#pragma once

#include <gtkmm.h>

#include "../core/board.h"
#include "board-card-button.h"
#include "board-widget.h"

namespace ui {

class BoardWidget;
class CreateBoardDialog;
class PreferencesBoardDialog;

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
class ProgressWindow : public Gtk::Window {
public:
    ProgressWindow(BaseObjectType* cobject,
                   const Glib::RefPtr<Gtk::Builder>& builder);
    ~ProgressWindow() override;

    void add_board(std::string board_filepath);

    /**
     * @brief Shows application information
     */
    void show_about_dialog();

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

private:
    const Glib::RefPtr<Gtk::Builder> window_builder;

    bool on_delete_mode = false;

    ui::ProgressAboutDialog about_dialog;
    ui::CreateBoardDialog* create_board_dialog;
    ui::PreferencesBoardDialog* preferences_board_dialog;
    ui::DeleteBoardsBar delete_boards_bar;
    ui::BoardWidget board_widget;

    void setup_menu_button();
    bool on_window_close();
};
}  // namespace ui