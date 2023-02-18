#include "app_info.h"
#include "app_window.h"
#include <gtkmm/label.h>
#include <gtkmm/aboutdialog.h>
#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>
#include <iostream>

ApplicationWindow::ApplicationWindow() :
header_bar(),
menu_button() {
    set_title("Progress Tracker");
    set_default_size(640, 480);
    set_size_request(640, 480);

    // TODO Add headerbar
    header_bar.set_title_widget(*Gtk::make_managed<Gtk::Label>(get_title()));

    // Todo Create MenuButton with an about section which when clicked shows an about dialog
    auto menu = Gio::Menu::create();
    menu->append("About", "appmenu.about");
    auto options = Gio::SimpleActionGroup::create();
    options->add_action(
        "about", sigc::mem_fun(*this, &ApplicationWindow::show_about_dialog)
    );

    menu_button.set_menu_model(menu);
    menu_button.insert_action_group("appmenu", options);

    header_bar.pack_end(menu_button);

    set_titlebar(header_bar);
}

ApplicationWindow::~ApplicationWindow() {

}

void ApplicationWindow::show_about_dialog() {
    auto about_dialog = Gtk::make_managed<Gtk::AboutDialog>();
    about_dialog->set_program_name("Progress Tracker");
    about_dialog->set_comments("App for storing Kanban-style todo lists.");
    about_dialog->set_copyright("De Moura © All rights reserved.");
    about_dialog->set_license("MIT License\n\nCopyright (c) 2023 de Moura");
    std::string version_str = "";
    version_str.append(std::to_string(MAJOR_VERSION));
    version_str.append(".");
    version_str.append(std::to_string(MINOR_VERSION));
    about_dialog->set_version(version_str);

    about_dialog->set_visible();
}