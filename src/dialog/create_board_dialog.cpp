#include "create_board_dialog.h"

#include <glibmm/i18n.h>
#include <utils.h>

namespace ui {
CreateBoardDialog::CreateBoardDialog(
    BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder,
    ProgressWindow& board_creator)
    : BoardDialog{cobject, ref_builder}, board_creator{board_creator} {
    set_title(_("Create Board"));
    p_left_button->add_css_class("destructive-action");
    p_right_button->add_css_class("suggested-action");

    p_right_button->set_label(_("Create"));

    /**
     * TODO: Create a helper class BoardManager. There is no need for having
     * CreateBoardDialog connected to the parent window
     */
    p_right_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CreateBoardDialog::create_board));
}

CreateBoardDialog* CreateBoardDialog::create(ProgressWindow& board_creator) {
    auto builder = Gtk::Builder::create_from_resource(BOARD_RESOURCE);

    return Gtk::Builder::get_widget_derived<CreateBoardDialog>(
        builder, "create-board", board_creator);
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
    std::string new_file_path = gen_unique_filename(board.get_name());
    if (!board.set_filepath(new_file_path)) {
        auto message_dialog = Gtk::AlertDialog::create(
            _("It was not possible to create a Board with given name"));
        message_dialog->show(*this);
    } else {
        board.save_as_xml();
        board_creator.add_board(new_file_path);
        close_window();
    }
}
}  // namespace ui
