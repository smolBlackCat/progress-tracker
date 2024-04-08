#include "create_board_dialog.h"

#include <glibmm/i18n.h>

#include <format>

#include "application.h"

namespace ui {
CreateBoardDialog::CreateBoardDialog(
    BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder,
    ProgressWindow& app_window)
    : BoardDialog{cobject, ref_builder}, app_window{app_window} {
    set_title(_("Create Board"));
    p_left_button->add_css_class("destructive-action");
    p_right_button->add_css_class("suggested-action");

    p_right_button->set_label(_("Create"));
    p_right_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CreateBoardDialog::create_board));
}

void CreateBoardDialog::create_board() {
    if (p_board_name_entry->get_text_length() == 0) {
        auto message_dialog =
            Gtk::AlertDialog::create(_("Empty board names are not allowed"));
        message_dialog->show(*this);
        return;
    }

    if ((!file_selected) &&
        p_background_selector_stack->get_visible_child_name() == "as-file") {
        auto message_dialog =
            Gtk::AlertDialog::create(_("A file must be selected"));
        message_dialog->show(*this);
        return;
    }

    std::string background_type =
        p_background_selector_stack->get_visible_child_name();
    std::string background = background_type == "as-file"
                                 ? p_select_file_label->get_text()
                                 : p_colour_button->get_rgba().to_string();
    Board board = Board{p_board_name_entry->get_text(), background};
    std::string new_file_path = Board::new_filename(board.get_name());
    if (!board.set_filepath(new_file_path)) {
        auto message_dialog = Gtk::AlertDialog::create(
            _("It was not possible to create a Board with given name"));
        message_dialog->show(*this);
    } else {
        board.save_as_xml();
        app_window.add_board(new_file_path);
        close_window();
    }
}
}  // namespace ui