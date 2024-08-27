#pragma once

#include <widgets/board-widget.h>

#include "board-dialog.h"

namespace ui {

/**
 * @brief Window dialog presenting the current settings of a board
 */
class PreferencesBoardDialog : public BoardDialog {
public:
    PreferencesBoardDialog(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& builder,
                           Gtk::Window& parent, BoardWidget& board_widget);

    ~PreferencesBoardDialog() override;

    /**
     * @brief Opens and loads the current board settings
     */
    void open_window() override;

    /**
     * @brief Creates a Board settings dialog for modification
     *
     * @param parent parent the dialog is transient for
     *
     * @return an object pointer to a PreferencesBoardDialog instance
     */
    static PreferencesBoardDialog* create(Gtk::Window& parent,
                                          BoardWidget& board_widget);

protected:
    BoardWidget& board_widget;
    Gtk::Window& parent;
    void load_board();
    void on_save_changes();
};
}  // namespace ui
