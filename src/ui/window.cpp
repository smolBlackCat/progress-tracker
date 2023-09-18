#include "window.h"

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <iostream>

namespace ui {

ProgressAboutDialog::ProgressAboutDialog(Gtk::Window& parent) {
    set_program_name("Progress");
    set_logo(Gdk::Texture::create_from_resource("/ui/com.moura.Progress.svg"));
    set_version("1.0");
    set_comments("Simple app for storing kanban-style todo lists");
    set_license_type(Gtk::License::MIT_X11);
    set_copyright("De Moura © All rights reserved");
    std::vector<Glib::ustring> authors{};
    authors.push_back("De Moura");
    set_authors(authors);

    set_hide_on_close();
    set_modal();
    set_transient_for(parent);
}

ProgressAboutDialog::~ProgressAboutDialog() {}

ProgressWindow::ProgressWindow()
    : header_bar(),
      add_board_button(),
      menu_button(),
      label("Add new board"),
      about_dialog(*this) {
    set_title("Progress");
    set_default_size(600, 600);
    set_size_request(600, 600);

    set_titlebar(header_bar);
    header_bar.pack_end(menu_button);
    header_bar.pack_start(add_board_button);

    // Set CreateBoardDialog
    auto builder =
        Gtk::Builder::create_from_resource("/ui/create-board-dialog.ui");
    create_board_dialog =
        Gtk::Builder::get_widget_derived<ui::CreateBoardDialog>(builder,
                                                                "create-board");
    create_board_dialog->set_transient_for(*this);

    add_board_button.set_icon_name("gtk-add");
    add_board_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::ProgressWindow::show_create_board));

    setup_menu_button();

    set_child(label);
}

ProgressWindow::~ProgressWindow() { delete create_board_dialog; }

void ProgressWindow::setup_menu_button() {
    auto action_group = Gio::SimpleActionGroup::create();
    action_group->add_action("about",
                             sigc::mem_fun(*this, &ProgressWindow::show_about));

    auto menu = Gio::Menu::create();
    menu->append("About", "win.about");

    menu_button.insert_action_group("win", action_group);
    menu_button.set_icon_name("open-menu");
    menu_button.set_menu_model(menu);
}

void ProgressWindow::show_about() {
    about_dialog.set_visible();
    about_dialog.present();
}

void ProgressWindow::show_create_board() { create_board_dialog->set_visible(); }
}  // namespace ui