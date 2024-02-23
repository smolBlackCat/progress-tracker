#pragma once

#include "../core/board.h"
#include "board-dialog.h"
#include "board-widget.h"

namespace ui {
/**
 * @brief Window dialog presenting the current settings of a board
 */
class PreferencesBoardDialog : public BoardDialog {
public:
    PreferencesBoardDialog(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& builder,
                           BoardWidget& board_widget, Gtk::Window& app_window);

    /**
     * @brief Sets a new a Board object from which the
     * preferences dialog will load and edit information from.
     * 
     * @details The pointer given always points to a dynamically allocated
     * object, therefore, it's safe to assume that nullptr won't be given.
     * 
     * @param board Board object pointer pointing to a dynamically allocated
     * object.
     */
    void set_board(Board* board);

protected:
    void close_window() override;

private:
    Board* board = nullptr;
    BoardWidget& board_widget;
    Gtk::Window& app_window;

    void on_save_changes();
};
}  // namespace ui