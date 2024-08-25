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
     * @brief Dialog Window constructor
     */
    CreateBoardDialog(BaseObjectType* cobject,
                      const Glib::RefPtr<Gtk::Builder>& ref_builder,
                      ProgressWindow& board_creator);

    /**
     * @brief Creates a Board settings dialog for creation
     *
     * @param board_creator helper object responsible for creating the board
     *
     * @return an object pointer to a CreateBoardDialog instance
     */
    static CreateBoardDialog* create(ProgressWindow& board_creator);

protected:
    ProgressWindow& board_creator;
    void create_board();
};
}  // namespace ui
