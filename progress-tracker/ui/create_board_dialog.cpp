#include "create_board_dialog.h"

#include <iostream>
#include <string>

ui::CreateBoardDialog::CreateBoardDialog()
    : header_bar(),
      root(Gtk::Orientation::VERTICAL),
      checkbuttons_box(Gtk::Orientation::VERTICAL),
      bg_label(),
      solid_colour_cbttn("Solid Colour"),
      image_cbttn("Image"),
      selected_bg_label("No colour selected"),
      select_bg_button("Select File"),
      create_button("Create"),
      cancel_button("Cancel"),
      board_name_entry(),
      dialog_colour_button(),
      dialog_data{{"title", ""}, {"background", ""}} {
    set_default_size(300, 400);
    set_size_request(300, 400);
    colour_dialog = Gtk::ColorDialog::create();
    alert_dialog = Gtk::AlertDialog::create("Board setting incomplete");
    this->setup_dialog_titlebar();
    this->setup_dialog_root();
    set_child(root);
}

std::map<std::string, std::string> ui::CreateBoardDialog::get_entry()
    const noexcept {
    return dialog_data;
}

void ui::CreateBoardDialog::on_bg_button_click() {
    /**
     * TODO: Code is not filtering the files properly, probably due to wrong
     * mime types.
     */
    auto dialog = Gtk::FileDialog::create();

    auto filters = Gio::ListStore<Gtk::FileFilter>::create();
    auto image_filter = Gtk::FileFilter::create();
    image_filter->set_name("Image Files");
    image_filter->add_mime_type("image/png");
    image_filter->add_mime_type("image/jpeg");
    image_filter->add_mime_type("image/tiff");
    filters->append(image_filter);
    dialog->set_filters(filters);

    dialog->open(sigc::bind(
        sigc::mem_fun(*this, &ui::CreateBoardDialog::on_filedialog_finish),
        dialog));
}

void ui::CreateBoardDialog::on_solid_radiobutton_toggle() {
    if (!solid_colour_cbttn.get_active()) return;

    selected_bg_label.set_visible(false);
    select_bg_button.set_visible(false);

    dialog_colour_button.set_visible();
}

void ui::CreateBoardDialog::on_image_radiobutton_toggle() {
    if (!image_cbttn.get_active()) return;

    dialog_colour_button.set_visible(false);

    select_bg_button.set_visible();
    selected_bg_label.set_visible();
    if (selected_file) {
        selected_bg_label.set_text(selected_file->get_basename());
    } else {
        selected_bg_label.set_text("No file selected");
    }
}

void ui::CreateBoardDialog::close_window() {
    set_visible(false);

    // Cleanup any inserted data
    dialog_colour_button.set_rgba(Gdk::RGBA());
    selected_bg_label.set_label("");
    board_name_entry.set_text("");
}

void ui::CreateBoardDialog::save_entry() {
    if (board_name_entry.get_text_length() == 0) {
        alert_dialog->set_detail("Board name should not be empty");
        alert_dialog->show(*this);
        return;
    } else {
        dialog_data["title"] = board_name_entry.get_text();
    }

    if (solid_colour_cbttn.get_active()) {
        if (dialog_colour_button.get_rgba().is_clear()) {
            alert_dialog->set_detail("No colour was selected");
            alert_dialog->show(*this);
            return;
        }
        dialog_data["background"] = dialog_colour_button.get_rgba().to_string();
    } else if (image_cbttn.get_active()) {
        if (!selected_file) {
            alert_dialog->set_detail("No file was selected");
            alert_dialog->show(*this);
            return;
        }
        dialog_data["background"] = selected_file->get_path();
    }

    close_window();
}

void ui::CreateBoardDialog::setup_dialog_root() {
    board_name_entry.set_placeholder_text("Board Name");
    board_name_entry.set_margin(4);
    root.append(board_name_entry);

    bg_label.set_markup("<span size=\"large\"><b>Background</b></span>");
    bg_label.set_halign(Gtk::Align::START);
    bg_label.set_margin_start(4);
    bg_label.set_margin_top(20);
    bg_label.set_margin_bottom(8);
    root.append(bg_label);

    solid_colour_cbttn.signal_toggled().connect(sigc::mem_fun(
        *this, &ui::CreateBoardDialog::on_solid_radiobutton_toggle));
    image_cbttn.signal_toggled().connect(sigc::mem_fun(
        *this, &ui::CreateBoardDialog::on_image_radiobutton_toggle));
    select_bg_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::CreateBoardDialog::on_bg_button_click));

    solid_colour_cbttn.set_active();
    image_cbttn.set_group(solid_colour_cbttn);

    dialog_colour_button.set_dialog(colour_dialog);

    checkbuttons_box.set_margin(10);
    checkbuttons_box.append(solid_colour_cbttn);
    checkbuttons_box.append(dialog_colour_button);
    checkbuttons_box.append(image_cbttn);
    checkbuttons_box.append(select_bg_button);
    checkbuttons_box.append(selected_bg_label);
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
        auto colour = colour_dialog->choose_rgba_finish(result);
        dialog_colour_button.set_rgba(colour);
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

        selected_bg_label.set_label(selected_file->get_basename());
    } catch (Gtk::DialogError& err) {
        err.what();
    } catch (Glib::Error& err) {
        err.what();
    }
}