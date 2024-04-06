#include "preferences-board-dialog.h"

#include <filesystem>

#include "i18n.h"

namespace ui {

PreferencesBoardDialog::PreferencesBoardDialog(
    BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder,
    BoardWidget& board_widget)
    : BoardDialog{cobject, builder}, board_widget{board_widget} {
    set_title(_("Edit Board"));
    p_right_button->set_label(_("Save"));
    p_right_button->add_css_class("suggested-action");

    p_right_button->signal_clicked().connect(
        sigc::mem_fun(*this, &PreferencesBoardDialog::on_save_changes));
}

void PreferencesBoardDialog::load_board() {
    p_board_name_entry->set_text(board_widget.get_board_name());
    if (Board::get_background_type(board_widget.get_background()) == "file") {
        selected_file =
            Gio::File::create_for_path(board_widget.get_background());
        p_file_image->set(board_widget.get_background());
        p_select_file_label->set_text(board_widget.get_background());
        file_selected = true;
    } else {
        // If this board doesn't have a solid colour as background,
        // it implies that no file was selected. Then clean everything
        // from other boards
        file_selected = false;
        p_select_file_label->set_label(_("No file was selected"));
        p_file_image->set("");

        p_colour_button->set_rgba(Gdk::RGBA{board_widget.get_background()});
        selected_colour.set(board_widget.get_background());
    }
}

void PreferencesBoardDialog::open_window() {
    BoardDialog::open_window();
    load_board();
}

void PreferencesBoardDialog::on_save_changes() {
    if (p_background_selector_stack->get_visible_child_name() == "as-file") {
        if (!file_selected) {
            auto message_dialog = Gtk::AlertDialog::create(_("Select a file"));
            message_dialog->show(*this);
            // The information is not saved in disk.
            return;
        }
        board_widget.set_background(selected_file->get_path());
    } else {
        board_widget.set_background(selected_colour.to_string());
    }
    board_widget.set_board_name(p_board_name_entry->get_text());

    std::string previous_filepath = board_widget.get_filepath();
    board_widget.set_filepath(
        Board::new_filename(p_board_name_entry->get_text()));
    std::filesystem::remove(previous_filepath);
    board_widget.save(false);

    close_window();
}
}  // namespace ui