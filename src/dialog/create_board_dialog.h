#pragma once

#include <gtkmm.h>

#include <string>

#include "../window.h"
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
                      ProgressWindow& app_window);

private:
    ProgressWindow& app_window;

    void create_board();
};
}  // namespace ui