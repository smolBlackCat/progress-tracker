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
                           BoardWidget& board_widget);

    void open_window() override;

private:
    BoardWidget& board_widget;

    void load_board();
    void on_save_changes();
};
}  // namespace ui
