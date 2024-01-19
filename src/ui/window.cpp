#include "window.h"

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <iostream>

#include "board-card-button.h"

namespace ui {

// TODO: Remove hardcoded text. Add support for other languages
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
      home_button{},
      menu_button(),
      about_dialog(*this),
      current_page{"board_grid"},
      board_grid{},
      board_widget{},
      stack{} {
    set_title("Progress");
    set_default_size(600, 600);
    set_size_request(600, 600);

    set_titlebar(header_bar);
    header_bar.pack_end(menu_button);
    header_bar.pack_start(home_button);
    home_button.set_visible(false);
    header_bar.pack_start(add_board_button);

    auto builder =
        Gtk::Builder::create_from_resource("/ui/create-board-dialog.ui");
    create_board_dialog =
        Gtk::Builder::get_widget_derived<ui::CreateBoardDialog>(builder,
                                                                "create-board");
    create_board_dialog->set_transient_for(*this);

    // Load application's stylesheet
    auto css_bytes = Gio::Resource::lookup_data_global("/ui/stylesheet.css");
    gsize size = css_bytes->get_size();
    std::string app_style = (char*)css_bytes->get_data(size);
    auto css_provider = Gtk::CssProvider::create();
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_provider->load_from_data(app_style.c_str());

    add_board_button.set_icon_name("gtk-add");
    add_board_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::ProgressWindow::show_create_board));

    home_button.set_icon_name("go-home");
    home_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ProgressWindow::go_to_main_menu));

    setup_menu_button();

    board_grid.set_margin(10);
    board_grid.set_valign(Gtk::Align::START);
    board_grid.set_expand(false);
    board_grid.set_selection_mode();

    Gtk::ScrolledWindow scrl_window{};
    scrl_window.set_child(board_grid);

    stack.add(scrl_window, "board_grid");
    stack.add(board_widget, "main_board");
    stack.set_transition_type(Gtk::StackTransitionType::SLIDE_LEFT_RIGHT);

    set_child(stack);
}

ProgressWindow::~ProgressWindow() { delete create_board_dialog; }

void ProgressWindow::add_board(Board* board) {
    auto new_board_card = Gtk::make_managed<BoardCardButton>(board);
    new_board_card->signal_clicked().connect([this, board]() {
        stack.set_visible_child("main_board");
        board_widget.set(board);
        home_button.set_visible();
        add_board_button.set_visible(false);
        set_title(board->get_name());
    });
    board_grid.append(*new_board_card);
}

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

void ProgressWindow::show_about() { about_dialog.set_visible(); }

void ProgressWindow::show_create_board() { create_board_dialog->set_visible(); }

void ProgressWindow::go_to_main_board() {
    stack.set_visible_child("main_board");
}

void ProgressWindow::go_to_main_menu() {
    stack.set_visible_child("board_grid");
    home_button.set_visible(false);
    add_board_button.set_visible();
    set_title("Progress");
}
}  // namespace ui