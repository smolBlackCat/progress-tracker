#include "window.h"

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <iostream>

namespace ui {

ProgressAboutDialog::ProgressAboutDialog(Gtk::Window& parent) {
    set_program_name("Progress");
    set_logo_icon_name("com.moura.ProgressTracker");
    set_version("1.0");
    set_comments("Simple app for storing kanban-style todo lists");
    set_license_type(Gtk::License::MIT_X11);
    set_copyright("De Moura © All rights reserved");
    std::vector<Glib::ustring> authors{};
    authors.push_back("De Moura");
    set_authors(authors);

    set_hide_on_close();
    set_transient_for(parent);
}

ProgressAboutDialog::~ProgressAboutDialog() {}

ProgressWindow::ProgressWindow()
    : header_bar(),
      add_board_button(),
      menu_button(),
      label("Add new board"),
      dialog(*this) {
    set_title("Progress");
    set_default_size(600, 600);
    set_size_request(600, 600);

    set_titlebar(header_bar);
    header_bar.set_title_widget(*Gtk::make_managed<Gtk::Label>(get_title()));
    header_bar.pack_end(menu_button);
    header_bar.pack_start(add_board_button);

    add_board_button.set_icon_name("gtk-add");

    setup_menu_button();

    set_child(label);
}

ProgressWindow::~ProgressWindow() {}

void ProgressWindow::setup_menu_button() {
    auto action_group = Gio::SimpleActionGroup::create();
    action_group->add_action("about",
                             sigc::mem_fun(*this, &ProgressWindow::show_about));

    auto menu = Gio::Menu::create();
    menu->append("About", "win.about");

    menu_button.insert_action_group("win", action_group);
    menu_button.set_menu_model(menu);
}

void ProgressWindow::show_about() {
    dialog.set_visible();
    dialog.present();
}
}  // namespace ui