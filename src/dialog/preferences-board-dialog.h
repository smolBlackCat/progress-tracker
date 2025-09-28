#pragma once

#include <widgets/board-widget.h>

#include "board-dialog.h"

namespace ui {

/**
 * @brief Window dialog presenting the current settings of a board.
 *
 * The PreferencesBoardDialog class provides a user interface for viewing and
 * modifying the current settings of a board. It extends the BoardDialog class
 * and adds specific functionalities for board preferences.
 */
class PreferencesBoardDialog : public BoardDialog {
public:
    /**
     * @brief Creates a PreferencesBoardDialog instance.
     *
     * @param board_widget Reference to the BoardWidget associated with this
     * dialog.
     *
     * @return Pointer to a PreferencesBoardDialog instance.
     */
    static PreferencesBoardDialog* create(BoardWidget& board_widget);

    void open(Gtk::Window& parent) override;

    /**
     * @brief Destructor.
     */
    ~PreferencesBoardDialog() override;

    /**
     * @brief Handles the footer button click action.
     *
     * This method is called when the footer button is clicked. It is
     * responsible for saving the changes made to the board settings.
     */
    void on_footer_button_click() override;

protected:
    /**
     * @brief Constructs a PreferencesBoardDialog object.
     *
     * @param board_widget Reference to the BoardWidget associated with this
     * dialog.
     */
    PreferencesBoardDialog(BoardWidget& board_widget);

    /**
     * @brief Loads the current settings of the board.
     *
     * This method is responsible for loading the current settings of the board
     * into the dialog.
     */
    void load_board();

    /**
     * @brief Saves the changes made to the board settings.
     *
     * This method is responsible for saving the changes made to the board
     * settings.
     */
    void on_save_changes();

    BoardWidget& board_widget;  ///< Reference to the BoardWidget associated
                                ///< with this dialog.
};

}  // namespace ui