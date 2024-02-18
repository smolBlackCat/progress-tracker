#include "window.h"

#include <filesystem>
#include <format>
#include <iostream>

#include "board-card-button.h"
#include "i18n.h"

namespace ui {

// TODO: Remove hardcoded text. Add support for other languages
ProgressAboutDialog::ProgressAboutDialog(Gtk::Window& parent) {
    set_program_name("Progress");
    set_logo(Gdk::Texture::create_from_resource("/ui/com.moura.Progress.svg"));
    set_version("1.0");
    set_comments(_("Simple app for storing kanban-style todo lists"));
    set_license_type(Gtk::License::MIT_X11);
    set_copyright(_("De Moura © All rights reserved"));
    std::vector<Glib::ustring> authors{};
    authors.push_back("De Moura");
    set_authors(authors);

    set_hide_on_close();
    set_modal();
    set_transient_for(parent);
}

ProgressAboutDialog::~ProgressAboutDialog() {}

DeleteBoardsBar::DeleteBoardsBar(WindowController& window_controller)
    : Gtk::Revealer{},
      root{Gtk::Orientation::HORIZONTAL},
      bar_text{_("Select the boards to be deleted")},
      bar_button_delete{_("Delete")},
      bar_button_cancel{_("Cancel")},
      window_controller{window_controller} {
    set_child(root);
    set_name("delete-board-infobar");
    set_transition_type(Gtk::RevealerTransitionType::SLIDE_UP);

    bar_text.set_margin(4);
    bar_text.set_markup(std::format("<b>{}</b>", bar_text.get_text().c_str()));
    bar_button_delete.set_margin(4);
    bar_button_delete.add_css_class("destructive-action");
    bar_button_cancel.set_margin(4);
    bar_button_cancel.add_css_class("suggested-action");
    root.append(bar_text);
    root.append(bar_button_delete);
    root.append(bar_button_cancel);
    root.set_spacing(4);

    bar_button_delete.signal_clicked().connect(sigc::mem_fun(
        window_controller, &WindowController::delete_selected_boards));
    bar_button_cancel.signal_clicked().connect(sigc::mem_fun(
        window_controller, &WindowController::off_delete_board_mode));
}

ProgressWindow::ProgressWindow(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window{cobject},
      window_builder{builder},
      board_widget{window_controller},
      create_board_dialog{
          Gtk::Builder::get_widget_derived<ui::CreateBoardDialog>(
              Gtk::Builder::create_from_resource("/ui/create-board-dialog.ui"),
              "create-board", window_controller)},
      preferences_board_dialog{
          Gtk::Builder::get_widget_derived<ui::PreferencesBoardDialog>(
              Gtk::Builder::create_from_resource("/ui/create-board-dialog.ui"),
              "create-board", board_widget)},
      about_dialog{*this},
      delete_boards_bar{window_controller},
      window_controller{
          window_builder, *create_board_dialog, *preferences_board_dialog,
          about_dialog,   board_widget,         delete_boards_bar} {
    create_board_dialog->set_transient_for(*this);
    preferences_board_dialog->set_transient_for(*this);

    // Load application's stylesheet
    auto css_bytes = Gio::Resource::lookup_data_global("/ui/stylesheet.css");
    gsize size = css_bytes->get_size();
    std::string app_style = (char*)css_bytes->get_data(size);
    auto css_provider = Gtk::CssProvider::create();
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_provider->load_from_data(app_style.c_str());

    builder->get_widget<Gtk::Button>("home-button")
        ->signal_clicked()
        .connect(
            sigc::mem_fun(window_controller, &WindowController::on_main_menu));
    builder->get_widget<Gtk::Button>("add-board-button")
        ->signal_clicked()
        .connect(sigc::mem_fun(window_controller,
                               &WindowController::show_create_board_dialog));
    setup_menu_button();

    delete_boards_bar.set_halign(Gtk::Align::CENTER);
    delete_boards_bar.set_valign(Gtk::Align::END);
    delete_boards_bar.set_vexpand();
    delete_boards_bar.set_margin_bottom(10);
    builder->get_widget<Gtk::Box>("root-box")->append(delete_boards_bar);
    builder->get_widget<Gtk::Stack>("app-stack")
        ->add(board_widget, "board-page");
}

WindowController& ProgressWindow::get_window_controller() {
    return window_controller;
}

ProgressWindow::~ProgressWindow() {
    delete create_board_dialog;
    delete preferences_board_dialog;
}

void ProgressWindow::setup_menu_button() {
    auto action_group = Gio::SimpleActionGroup::create();
    action_group->add_action(
        "about",
        sigc::mem_fun(window_controller, &WindowController::show_about_dialog));
    action_group->add_action(
        "delete", sigc::mem_fun(window_controller, &WindowController::on_delete_board_mode));
    action_group->add_action(
        "preferences", [this]() {
        preferences_board_dialog->open_window(); });

    window_builder->get_widget<Gtk::MenuButton>("app-menu-button")
        ->insert_action_group("win", action_group);
}
}  // namespace ui