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
                           BoardWidget& board_widget);
    
    void open_window() override;

private:
    BoardWidget& board_widget;

    void on_save_changes();
};
}  // namespace ui