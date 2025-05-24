#include "preferences-board-dialog.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <utils.h>

#include "adwaita.h"

namespace ui {

PreferencesBoardDialog::PreferencesBoardDialog(BoardWidget& board_widget)
    : BoardDialog{}, board_widget{board_widget} {
    adw_dialog_set_title(ADW_DIALOG(board_dialog->gobj()),
                         _("Board Preferences"));
    footer_button->set_label(_("Save"));
    load_board();
}

PreferencesBoardDialog::~PreferencesBoardDialog() {}

void PreferencesBoardDialog::load_board() {
    board_title_entry->set_text(board_widget.get_name());
    BackgroundType bg_type =
        Board::get_background_type(board_widget.get_background());
    switch (bg_type) {
        case BackgroundType::COLOR: {
            this->bg_type = bg_type;
            set_picture(Gdk::RGBA{board_widget.get_background()});
            break;
        }
        case BackgroundType::IMAGE: {
            this->bg_type = bg_type;
            image_filename = board_widget.get_background();
            set_picture(image_filename);
            break;
        }
        case BackgroundType::INVALID: {
            this->bg_type = BackgroundType::COLOR;
            spdlog::get("ui")->warn(
                "[PreferencesBoardDialog] Current board background is invalid. "
                "Falling back to default");
            set_picture(Gdk::RGBA{});
            break;
        }
    }
}

PreferencesBoardDialog* PreferencesBoardDialog::create(
    BoardWidget& board_widget) {
    return new PreferencesBoardDialog(board_widget);
}

void PreferencesBoardDialog::on_footer_button_click() { on_save_changes(); }

void PreferencesBoardDialog::on_save_changes() {
    std::string new_name = board_title_entry->get_text();
    parent->set_title(new_name);
    board_widget.set_name(new_name);

    if (new_name.empty()) {
        auto message_dialog =
            Gtk::AlertDialog::create(_("Empty board names are not allowed"));
        message_dialog->show(*parent);
        return;
    }

    switch (bg_type) {
        case BackgroundType::COLOR: {
            board_widget.set_background(rgba.to_string());
            break;
        }
        case BackgroundType::IMAGE: {
            board_widget.set_background(image_filename);
            break;
        }
        default: {
            // Report probable corruption
        }
    }
    board_widget.save(false);

    adw_dialog_close(ADW_DIALOG(board_dialog->gobj()));
}
}  // namespace ui
