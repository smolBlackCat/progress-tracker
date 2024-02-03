#include "preferences-board-dialog.h"

#include <filesystem>

namespace ui {

PreferencesBoardDialog::PreferencesBoardDialog(
    BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder,
    Board* board, BoardWidget& board_widget, Gtk::Window& app_window)
    : BoardDialog{cobject, builder},
      board{board},
      board_widget{board_widget},
      app_window{app_window} {
    set_board(board);

    set_title("Edit Board");
    p_right_button->set_label("Save");
    p_right_button->add_css_class("suggested-action");

    p_right_button->signal_clicked().connect(
        sigc::mem_fun(*this, &PreferencesBoardDialog::on_save_changes));
}

void PreferencesBoardDialog::set_board(Board* board) {
    if (board) {
        this->board = board;

        p_board_name_entry->set_text(board->get_name());
        if (board->get_background_type() == "file") {
            selected_file = Gio::File::create_for_path(board->get_background());
            p_file_image->set(board->get_background());
            p_select_file_label->set_text(board->get_background());
            file_selected = true;
        } else {
            // If this board doesn't have a solid colour as background,
            // it implies that no file was selected. Then clean everything
            // from other boards
            file_selected = false;
            p_select_file_label->set_label("No file was selected");
            p_file_image->set("");

            p_colour_button->set_rgba(Gdk::RGBA{board->get_background()});
            selected_colour.set(board->get_background());
        }
    }
}

void PreferencesBoardDialog::close_window() {
    BoardDialog::close_window();
    // Recover the information deleted by super
    set_board(board);
}

void PreferencesBoardDialog::on_save_changes() {
    if (board) {
        if (p_background_selector_stack->get_visible_child_name() ==
            "as-file") {
            if (!file_selected) {
                auto message_dialog = Gtk::AlertDialog::create("Select a file");
                message_dialog->show(*this);
                // The information is not saved in disk.
                return;
            }
            board->set_background(selected_file->get_path());
        } else {
            board->set_background(selected_colour.to_string());
        }
        board->set_name(p_board_name_entry->get_text());
        app_window.set_title(p_board_name_entry->get_text());
        board_widget.set_background();

        std::string previous_filepath = board->get_filepath();
        board->set_filepath(
            Board::new_filename(p_board_name_entry->get_text()));
        std::filesystem::remove(previous_filepath);
        board->save_as_xml();
        board_widget.board_card_button->update(board);
    }

    close_window();
}
}  // namespace ui