#pragma once

#include <widgets/board-widget.h>

#include "board-dialog.h"

namespace ui {

/**
 * @brief Window dialog presenting the current settings of a board
 */
class PreferencesBoardDialog : public BoardDialog {
public:
    static PreferencesBoardDialog* create(BoardWidget& board_widget);
    ~PreferencesBoardDialog() override;

    void on_footer_button_click() override;

protected:
    PreferencesBoardDialog(BoardWidget& board_widget);
    BoardWidget& board_widget;
    void load_board();
    void on_save_changes();
};
}  // namespace ui
