#include "create_board_dialog.h"

#include <glibmm/i18n.h>
#include <utils.h>

namespace ui {
CreateBoardDialog::CreateBoardDialog(ProgressWindow& board_creator)
    : BoardDialog{}, board_creator{board_creator} {
    adw_dialog_set_title(ADW_DIALOG(board_dialog->gobj()),
                         _("Create New Board"));
}

CreateBoardDialog* CreateBoardDialog::create(ProgressWindow& board_creator) {
    return new CreateBoardDialog(board_creator);
}

void CreateBoardDialog::on_footer_button_click() { create_board(); }

void CreateBoardDialog::create_board() {
    if (board_title_entry->get_text_length() == 0) {
        auto message_dialog =
            Gtk::AlertDialog::create(_("Empty board names are not allowed"));
        message_dialog->show(*parent);
        return;
    }

    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath",
                          gen_unique_filename(board_title_entry->get_text()));

    switch (bg_type) {
        case BackgroundType::COLOR: {
            // Not a clean solution, we have to rely on calculation everytime
            // because there is no exact standard for colour setting
            auto rgb_string = std::format(
                "rgb({},{},{})", (int)(rgba.get_red() * 255),
                (int)(rgba.get_green() * 255), (int)(rgba.get_blue() * 255));
            Board board =
                backend.create(board_title_entry->get_text(), rgb_string);
            board.save();
            break;
        }
        case BackgroundType::IMAGE: {
            Board board =
                backend.create(board_title_entry->get_text(), image_filename);
            board.save();
            break;
        }
        default: {
            // Report probable data corruption
            break;
        }
    }

    board_creator.add_local_board(backend);
    adw_dialog_close(ADW_DIALOG(board_dialog->gobj()));
}
}  // namespace ui
