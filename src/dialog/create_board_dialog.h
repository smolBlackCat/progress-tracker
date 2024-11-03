#pragma once

#include <gtkmm.h>
#include <window.h>

#include "board-dialog.h"

namespace ui {
/**
 * @class CreateBoardDialog
 *
 * @brief Custom dialog window for creating new boards.
 */
class CreateBoardDialog : public BoardDialog {
public:

    /**
     * @brief Creates a Board settings dialog for creation
     *
     * @param board_creator helper object responsible for creating the board
     *
     * @return an object pointer to a CreateBoardDialog instance
     */
    static CreateBoardDialog* create(ProgressWindow& board_creator);

    void on_footer_button_click() override;

protected:
    CreateBoardDialog(ProgressWindow& board_creator);

    ProgressWindow& board_creator;
    void create_board();
};
}  // namespace ui
