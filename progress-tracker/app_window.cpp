#include "app_info.h"
#include "app_window.h"
#include <gtkmm/label.h>
#include <gtkmm/aboutdialog.h>
#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>
#include <gtk/gtkaboutdialog.h>
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
    gtk_show_about_dialog(
        this->gobj(),
        "program_name", "Progress",
        "version", "1.0.0",
        "comments", "Simple app for storing kanban-style todo lists.",
        "license_type", GtkLicense::GTK_LICENSE_MIT_X11,
        "logo_icon_name", "com.moura.ProgressTracker",
        nullptr
    );
}
