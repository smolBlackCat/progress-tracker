#include "preferences-board-dialog.h"

#include <glibmm/i18n.h>
#include <utils.h>

#include <filesystem>

namespace ui {

PreferencesBoardDialog::PreferencesBoardDialog(
    BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder,
    Gtk::Window& parent, BoardWidget& board_widget)
    : BoardDialog{cobject, builder},
      board_widget{board_widget},
      parent{parent} {
    set_title(_("Edit Board"));
    set_transient_for(parent);

    p_right_button->set_label(_("Save"));
    p_right_button->add_css_class("suggested-action");
    p_right_button->signal_clicked().connect(
        sigc::mem_fun(*this, &PreferencesBoardDialog::on_save_changes));
}

PreferencesBoardDialog::~PreferencesBoardDialog() {}

void PreferencesBoardDialog::load_board() {
    p_board_name_entry->set_text(board_widget.get_name());
    BackgroundType bg_type =
        Board::get_background_type(board_widget.get_background());
    switch (bg_type) {
        case BackgroundType::COLOR: {
            file_selected = false;
            p_select_file_label->set_label(_("No file was selected"));
            p_file_image->set("");

            p_colour_button->set_rgba(Gdk::RGBA{board_widget.get_background()});
            selected_colour.set(board_widget.get_background());
            break;
        }
        case BackgroundType::IMAGE: {
            file_selected = true;
            selected_file =
                Gio::File::create_for_path(board_widget.get_background());
            p_file_image->set(board_widget.get_background());
            p_select_file_label->set_text(board_widget.get_background());
            break;
        }
        case BackgroundType::INVALID: {
            file_selected = false;
            p_select_file_label->set_label(_("No file was selected"));
            p_file_image->set("");

            p_colour_button->set_rgba(Gdk::RGBA{board_widget.get_background()});
            selected_colour.set(board_widget.get_background());
            break;
        }
    }
}

PreferencesBoardDialog* PreferencesBoardDialog::create(
    Gtk::Window& parent, BoardWidget& board_widget) {
    auto builder = Gtk::Builder::create_from_resource(BOARD_RESOURCE);
    auto preferences_board_dialog =
        Gtk::Builder::get_widget_derived<PreferencesBoardDialog>(
            builder, "create-board", parent, board_widget);

    return preferences_board_dialog;
}

void PreferencesBoardDialog::open_window() {
    BoardDialog::open_window();
    load_board();
}

void PreferencesBoardDialog::on_save_changes() {
    if (p_background_selector_stack->get_visible_child_name() == "as-file") {
        if (!file_selected) {
            Gtk::AlertDialog::create(_("Select a file"))->show(*this);
            return;
        }
        board_widget.set_background(selected_file->get_path());
    } else {
        board_widget.set_background(selected_colour.to_string());
    }

    const std::string& previous_name = board_widget.get_name();
    std::string new_name = p_board_name_entry->get_text();
    if (previous_name != new_name) {
        board_widget.set_name(new_name);
        parent.set_title(new_name);
        std::string previous_filepath = board_widget.get_filepath();
        board_widget.set_filepath(gen_unique_filename(new_name));
        std::filesystem::remove(previous_filepath);
    }

    board_widget.save(false);
    close_window();
}
}  // namespace ui
