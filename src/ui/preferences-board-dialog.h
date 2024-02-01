#pragma once

#include "../core/board.h"
#include "board-widget.h"
#include "board-dialog.h"

namespace ui {
/**
 * @brief Window dialog presenting the current settings of a board
*/
class PreferencesBoardDialog : public BoardDialog {
public:
    PreferencesBoardDialog(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& builder,
                           Board* board, BoardWidget& board_widget, Gtk::Window& app_window);

    void set_board(Board* board);

protected:
    void close_window() override;

private:
    Board* board;
    BoardWidget& board_widget;
    Gtk::Window& app_window;

    void on_save_changes();
};
}  // namespace ui