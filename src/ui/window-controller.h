#pragma once

#include <gtkmm.h>

#include "board-dialog.h"
#include "board-widget.h"

namespace ui {

class DeleteBoardsBar;

/**
 * @brief Class implementing behaviours done by the application's window
 */
class WindowController {
public:
    WindowController(const Glib::RefPtr<Gtk::Builder>& window_builder,
                     Gtk::Window& cb_dialog, Gtk::Window& pref_dialog,
                     Gtk::Window& about_dialog,
                     BoardWidget& board_widget,
                     DeleteBoardsBar& delete_boards_bar);

    /**
     * @brief Adds a board to the application's board grid.
     *
     * @param board_filepath Board's location in the system
     */
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

    /**
     * @brief Sets the window title
     */
    void set_title(std::string title);

protected:
    const Glib::RefPtr<Gtk::Builder>& window_builder;
    Gtk::Window &cb_dialog, &pref_dialog, &about_dialog;
    DeleteBoardsBar& delete_boards_bar;
    BoardWidget& board_widget;
    bool on_delete_mode = false;
};
}  // namespace ui
