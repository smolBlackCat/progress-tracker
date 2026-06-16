#include "preferences-board-dialog.h"

#include <adwaita.h>
#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <utils.h>

namespace ui {

PreferencesBoardDialog::PreferencesBoardDialog(BoardWidget& board_widget)
    : BoardDialog{}, board_widget{board_widget} {
    adw_dialog_set_title(ADW_DIALOG(board_dialog->gobj()),
                         _("Board Preferences"));
    footer_button->set_label(_("Save"));
}

PreferencesBoardDialog::~PreferencesBoardDialog() {}

void PreferencesBoardDialog::load_board() {
    std::string board_name = board_widget.get_name();
    std::string board_background = board_widget.get_background();

    board_title_entry->set_text(board_name);
    BackgroundType bg_type = Board::get_background_type(board_background);
    switch (bg_type) {
        case BackgroundType::COLOR: {
            this->bg_type = bg_type;
            set_picture(Gdk::RGBA{board_background});
            break;
        }
        case BackgroundType::IMAGE: {
            this->bg_type = bg_type;
            image_filename = board_background;
            set_picture(image_filename);
            break;
        }
        case BackgroundType::INVALID: {
            this->bg_type = BackgroundType::COLOR;
            spdlog::get("ui")->warn(
                "[PreferencesBoardDialog.load_board] Cannot load Board "
                "(\"{}\") background. Falling back to default",
                board_name);
            set_picture(Gdk::RGBA{});
            break;
        }
    }
}

PreferencesBoardDialog* PreferencesBoardDialog::create(
    BoardWidget& board_widget) {
    return new PreferencesBoardDialog(board_widget);
}

void PreferencesBoardDialog::open(Gtk::Window& parent) {
    BoardDialog::open(parent);
    load_board();
}

void PreferencesBoardDialog::on_footer_button_click() { on_save_changes(); }

void PreferencesBoardDialog::on_save_changes() {
    const std::string new_name = board_title_entry->get_text();

    if (new_name.empty()) {
        auto message_dialog =
            Gtk::AlertDialog::create(_("Empty board names are not allowed"));
        message_dialog->show(*parent);
        return;
    }

    board_widget.set_name(new_name);

    std::string new_background;
    switch (bg_type) {
        case BackgroundType::COLOR: {
            new_background = rgba.to_string();
            break;
        }
        case BackgroundType::IMAGE: {
            new_background = image_filename;
            break;
        }
        default: {
            // Report probable corruption
        }
    }

    board_widget.set_background(new_background);

    adw_dialog_close(ADW_DIALOG(board_dialog->gobj()));
}
}  // namespace ui
