#pragma once

#include <core/board-manager.h>
#include <gtkmm.h>
#include <window.h>

#include "board-dialog.h"

namespace ui {

/**
 * @class CreateBoardDialog
 *
 * @brief Custom dialog window for creating new boards.
 *
 * The CreateBoardDialog class provides a user interface for creating new
 * boards. It extends the BoardDialog class and adds specific functionalities
 * for board creation.
 */
class CreateBoardDialog : public BoardDialog {
public:
    /**
     * @brief Creates a Board settings dialog for creation.
     *
     * @param board_creator Reference to the ProgressWindow responsible for
     * creating the board.
     *
     * @return Pointer to a CreateBoardDialog instance.
     */
    static CreateBoardDialog* create(BoardManager& board_creator);

    /**
     * @brief Handles the footer button click action.
     *
     * This method is called when the footer button is clicked. It is
     * responsible for handling the creation of the new board.
     */
    void on_footer_button_click() override;

protected:
    /**
     * @brief Constructs a CreateBoardDialog object.
     *
     * @param board_creator Reference to the ProgressWindow responsible for
     * creating the board.
     */
    CreateBoardDialog(BoardManager& board_creator);

    /**
     * @brief Creates a new board.
     *
     * This method is responsible for creating a new board based on the user
     * input.
     */
    void create_board();

    BoardManager& m_manager;
};

}  // namespace ui