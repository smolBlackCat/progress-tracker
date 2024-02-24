#include "board-dialog.h"

#include "i18n.h"

namespace ui {
BoardDialog::BoardDialog(BaseObjectType* cobject,
                         const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window{cobject},
      p_board_name_entry{builder->get_widget<Gtk::Entry>("board-name-entry")},
      p_select_file_label{builder->get_widget<Gtk::Label>("select-file-label")},
      p_background_selector_stack{
          builder->get_widget<Gtk::Stack>("background-selector-stack")},
      p_colour_button{
          builder->get_widget<Gtk::ColorDialogButton>("colour-button")},
      p_file_image{builder->get_widget<Gtk::Image>("file-image")},
      p_left_button{builder->get_widget<Gtk::Button>("left-button")},
      p_right_button{builder->get_widget<Gtk::Button>("right-button")},
      p_select_file_button{
          builder->get_widget<Gtk::Button>("select-file-button")} {
    p_board_name_entry->set_placeholder_text(_("Board's name"));
    p_left_button->set_label(_("Cancel"));
    p_select_file_label->set_label(_("Select a file"));
    p_select_file_button->set_label(_("Select File"));

    p_left_button->signal_clicked().connect(
        sigc::mem_fun(*this, &BoardDialog::close_window));
    p_colour_button->property_rgba().signal_changed().connect(
        sigc::mem_fun(*this, &BoardDialog::on_colourbutton_set));
    p_select_file_button->signal_clicked().connect(
        sigc::mem_fun(*this, &BoardDialog::on_bg_button_click));
}

void BoardDialog::open_window() {
    set_visible();
}

void BoardDialog::close_window() {
    set_visible(false);

    // Cleanup any inserted data
    p_board_name_entry->set_text("");
    p_select_file_label->set_text(_("No file selected"));
    p_file_image->property_file().set_value("");
    p_colour_button->set_rgba(Gdk::RGBA("#FFFFFF"));
    file_selected = false;
}

void BoardDialog::on_bg_button_click() {
    auto dialog = Gtk::FileDialog::create();

    dialog->set_title(_("Select a file"));
    dialog->set_modal();

    auto filters = Gio::ListStore<Gtk::FileFilter>::create();
    auto image_filter = Gtk::FileFilter::create();
    image_filter->set_name(_("Image Files"));
    image_filter->add_mime_type("image/png");
    image_filter->add_mime_type("image/jpeg");
    image_filter->add_mime_type("image/tiff");
    filters->append(image_filter);
    dialog->set_filters(filters);

    dialog->open(
        *this,
        sigc::bind(sigc::mem_fun(*this, &ui::BoardDialog::on_filedialog_finish),
                   dialog));
}

void BoardDialog::on_filedialog_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result,
    const Glib::RefPtr<Gtk::FileDialog>& dialog) {
    try {
        selected_file = dialog->open_finish(result);
        p_file_image->property_paintable().set_value(
            Gdk::Texture::create_from_file(selected_file));
        p_select_file_label->set_text(selected_file->get_path());
        file_selected = true;
    } catch (Gtk::DialogError& err) {
        err.what();
    } catch (Glib::Error& err) {
        err.what();
    }
}

void BoardDialog::on_colourbutton_set() {
    selected_colour = p_colour_button->get_rgba();
}
}  // namespace ui