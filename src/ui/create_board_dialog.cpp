#include "create_board_dialog.h"

#include <iostream>
#include <string>

#include "window.h"

namespace ui {
CreateBoardDialog::CreateBoardDialog(
    BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder)
    : Gtk::Window(cobject), builder(ref_builder),
    p_colour_button(builder->get_widget<Gtk::ColorDialogButton>("colour-button")),
    p_file_image(builder->get_widget<Gtk::Image>("file-image")),
    p_select_file_label(builder->get_widget<Gtk::Label>("select-file-label")),
    p_board_name_entry(builder->get_widget<Gtk::Entry>("board-name-entry")),
    p_background_selector_stack(builder->get_widget<Gtk::Stack>("background-selector-stack")) {

    this->setup_dialog_buttons();
}

void CreateBoardDialog::on_bg_button_click() {
    /**
     * TODO: Code is not filtering the files properly, probably due to wrong
     * mime types.
     */
    auto dialog = Gtk::FileDialog::create();

    dialog->set_title("Select a file");
    dialog->set_modal();

    auto filters = Gio::ListStore<Gtk::FileFilter>::create();
    auto image_filter = Gtk::FileFilter::create();
    image_filter->set_name("Image Files");
    image_filter->add_mime_type("image/png");
    image_filter->add_mime_type("image/jpeg");
    image_filter->add_mime_type("image/tiff");
    filters->append(image_filter);
    dialog->set_filters(filters);

    dialog->open(*this, sigc::bind(
        sigc::mem_fun(*this, &ui::CreateBoardDialog::on_filedialog_finish),
        dialog));
}

// TODO: Implement cleanup code for whenever the dialog is closed
void CreateBoardDialog::close_window() {
    set_visible(false);

    // Cleanup any inserted data
    p_board_name_entry->set_text("");
    p_select_file_label->set_text("No file selected");
    p_file_image->property_file().set_value("");
    p_colour_button->set_rgba(Gdk::RGBA("#FFFFFF"));
}

void CreateBoardDialog::create_board() {
    std::string background_type = p_background_selector_stack->get_visible_child_name();
    std::string background = background_type == "as-file"?
        p_select_file_label->get_text():selected_colour.to_string();
    Board board{p_board_name_entry->get_text(), background};
    ((ui::ProgressWindow*) get_transient_for())->add_board(board);
    close_window();
}

void CreateBoardDialog::setup_dialog_buttons() {
    // Header Bar buttons
    builder->get_widget<Gtk::Button>("create-button")
        ->signal_clicked()
        .connect(sigc::mem_fun(*this, &ui::CreateBoardDialog::create_board));
    builder->get_widget<Gtk::Button>("cancel-button")
        ->signal_clicked()
        .connect(sigc::mem_fun(*this, &ui::CreateBoardDialog::close_window));

    // Stack Buttons
    builder->get_widget<Gtk::Button>("select-file-button")
        ->signal_clicked()
        .connect(
            sigc::mem_fun(*this, &ui::CreateBoardDialog::on_bg_button_click));
    builder->get_widget<Gtk::ColorDialogButton>("colour-button")
        ->property_rgba()
        .signal_changed()
        .connect(sigc::mem_fun(*this, &CreateBoardDialog::on_colourbutton_set));
}

void CreateBoardDialog::on_filedialog_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result,
    const Glib::RefPtr<Gtk::FileDialog>& dialog) {
    try {
        selected_file = dialog->open_finish(result);
        p_file_image->property_paintable().set_value(
            Gdk::Texture::create_from_file(selected_file));
        p_select_file_label->set_text(selected_file->get_path());
    } catch (Gtk::DialogError& err) {
        err.what();
    } catch (Glib::Error& err) {
        err.what();
    }
}

void CreateBoardDialog::on_colourbutton_set() {
    selected_colour = p_colour_button->get_rgba();
}
}  // namespace ui