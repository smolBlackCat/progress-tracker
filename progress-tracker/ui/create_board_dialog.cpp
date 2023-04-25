#include "create_board_dialog.h"

#include <iostream>

ui::CreateBoardDialog::CreateBoardDialog()
    : header_bar(),
      root(),
      checkbuttons_box(),
      bg_label(),
      solid_colour_cbttn("Solid Colour"),
      image_cbttn("Image"),
      selected_bg_label("No colour selected"),
      select_bg_button("Select Colour"),
      create_button("Create"),
      cancel_button("Cancel"),
      board_name_entry(),
      dialog_data{{"name", ""}, {"rgba", ""}} {
    set_default_size(300, 400);
    set_size_request(300, 400);
    colour_dialog = Gtk::ColorDialog::create();
    this->setup_dialog_titlebar();
    this->setup_dialog_root();
    set_child(root);
}

std::map<std::string, std::string> ui::CreateBoardDialog::get_entry()
    const noexcept {
    return dialog_data;
}

/**
 * TODO: Implement code that's executed for each button (background_button)
 */
void ui::CreateBoardDialog::on_bg_button_click() {
    if (solid_colour_cbttn.get_active()) {
        colour_dialog->choose_rgba(
            *this, selected_colour,
            sigc::mem_fun(*this,
                          &ui::CreateBoardDialog::on_colourdialog_finish));
    } else {
        // File chooser code

        // Create filters: jpeg, png tiff files only
        auto dialog = Gtk::FileDialog::create();

        auto filters = Gio::ListStore<Gtk::FileFilter>::create();
        auto image_filter = Gtk::FileFilter::create();
        image_filter->add_mime_type("image/png");
        image_filter->add_mime_type("image/jpeg");
        image_filter->add_mime_type("image/tiff");
        dialog->set_filters(filters);

        dialog->open(sigc::bind(
            sigc::mem_fun(*this, &ui::CreateBoardDialog::on_filedialog_finish),
            dialog));
    }
}

void ui::CreateBoardDialog::on_solid_radiobutton_toggle() {
    if (!solid_colour_cbttn.get_active()) return;

    checkbuttons_box.reorder_child_after(select_bg_button, solid_colour_cbttn);
    checkbuttons_box.reorder_child_after(selected_bg_label, select_bg_button);

    select_bg_button.set_label("Select Colour");
    selected_bg_label.set_text("No colour selected");
}

void ui::CreateBoardDialog::on_image_radiobutton_toggle() {
    if (!image_cbttn.get_active()) return;

    checkbuttons_box.reorder_child_after(select_bg_button, image_cbttn);
    checkbuttons_box.reorder_child_after(selected_bg_label, select_bg_button);

    select_bg_button.set_label("Select a file");
    selected_bg_label.set_text("No file selected");
}

void ui::CreateBoardDialog::close_window() {
    std::cout << selected_colour.to_string() << std::endl;
    set_visible(false);
}

void ui::CreateBoardDialog::save_entry() {}

void ui::CreateBoardDialog::setup_dialog_root() {
    root.set_orientation(Gtk::Orientation::VERTICAL);
    board_name_entry.set_placeholder_text("Board Name");
    board_name_entry.set_margin(4);
    root.append(board_name_entry);
    bg_label.set_markup("<span size=\"large\"><b>Background</b></span>");
    bg_label.set_halign(Gtk::Align::START);
    bg_label.set_margin_start(4);
    bg_label.set_margin_top(20);
    bg_label.set_margin_bottom(8);
    root.append(bg_label);

    checkbuttons_box.set_orientation(Gtk::Orientation::VERTICAL);
    solid_colour_cbttn.signal_toggled().connect(sigc::mem_fun(
        *this, &ui::CreateBoardDialog::on_solid_radiobutton_toggle));
    image_cbttn.signal_toggled().connect(sigc::mem_fun(
        *this, &ui::CreateBoardDialog::on_image_radiobutton_toggle));
    solid_colour_cbttn.set_active();
    image_cbttn.set_group(solid_colour_cbttn);
    select_bg_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::CreateBoardDialog::on_bg_button_click));
    checkbuttons_box.append(solid_colour_cbttn);
    checkbuttons_box.insert_child_after(select_bg_button, solid_colour_cbttn);
    checkbuttons_box.insert_child_after(selected_bg_label, select_bg_button);
    checkbuttons_box.append(image_cbttn);
    root.append(checkbuttons_box);
}

void ui::CreateBoardDialog::setup_dialog_titlebar() {
    create_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::CreateBoardDialog::save_entry));
    cancel_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::CreateBoardDialog::close_window));

    header_bar.pack_start(cancel_button);
    header_bar.pack_end(create_button);
    header_bar.set_show_title_buttons(false);
    auto dialog_title = Gtk::make_managed<Gtk::Label>();
    dialog_title->set_markup("<b>Create Board</b>");
    header_bar.set_title_widget(*dialog_title);

    set_titlebar(header_bar);
}

void ui::CreateBoardDialog::on_colourdialog_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        selected_colour = colour_dialog->choose_rgba_finish(result);
        selected_bg_label.set_label(selected_colour.to_string());
    } catch (const Gtk::DialogError& err) {
        err.what();
    } catch (const Glib::Error& err) {
        err.what();
    }
}

void ui::CreateBoardDialog::on_filedialog_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result,
    const Glib::RefPtr<Gtk::FileDialog>& dialog) {
    try {
        selected_file = dialog->open_finish(result);

        selected_bg_label.set_label(selected_file->get_path());
    } catch (Gtk::DialogError& err) {
        err.what();
    } catch (Glib::Error& err) {
        err.what();
    }
}