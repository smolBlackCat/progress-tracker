#include "create_board_dialog.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>

namespace ui {
CreateBoardDialog::CreateBoardDialog(BoardManager& board_creator)
    : BoardDialog{}, m_manager{board_creator} {
    adw_dialog_set_title(ADW_DIALOG(board_dialog->gobj()),
                         _("Create New Board"));
}

CreateBoardDialog* CreateBoardDialog::create(BoardManager& board_creator) {
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

    switch (bg_type) {
        case BackgroundType::COLOR: {
            // Not a clean solution, we have to rely on calculation everytime
            // because there is no exact standard for colour setting
            auto rgb_string = std::format(
                "rgb({},{},{})", (int)(rgba.get_red() * 255),
                (int)(rgba.get_green() * 255), (int)(rgba.get_blue() * 255));
            m_manager.local_add(board_title_entry->get_text(), rgb_string);
            break;
        }
        case BackgroundType::IMAGE: {
            m_manager.local_add(board_title_entry->get_text(), image_filename);
            break;
        }
        default: {
            // Report probable data corruption
            break;
        }
    }

    spdlog::get("ui")->info(
        "[CreateBoardDialog] Dialog has created a new board successfully");
    adw_dialog_close(ADW_DIALOG(board_dialog->gobj()));
}
}  // namespace ui
