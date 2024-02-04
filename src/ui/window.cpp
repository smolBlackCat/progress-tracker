#include "window.h"

#include <filesystem>
#include <format>
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

DeleteBoardsBar::DeleteBoardsBar(Gtk::FlowBox& boards_grid,
                                 ui::ProgressWindow& app_window)
    : Gtk::Revealer{},
      root{Gtk::Orientation::HORIZONTAL},
      bar_text{"Select the boards to be deleted"},
      bar_button_delete{"Delete"},
      bar_button_cancel{"Cancel"},
      boards_grid{boards_grid},
      app_window{app_window} {
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

    bar_button_delete.signal_clicked().connect(
        sigc::mem_fun(*this, &DeleteBoardsBar::on_delete_boards));
    bar_button_cancel.signal_clicked().connect(
        sigc::mem_fun(app_window, &ProgressWindow::off_delete_board));
}

void DeleteBoardsBar::on_delete_boards() {
    auto selected_children = boards_grid.get_selected_children();
    for (auto& fb_child_p : selected_children) {
        ui::BoardCardButton* cur_child =
            (ui::BoardCardButton*)fb_child_p->get_child();
        std::filesystem::remove(cur_child->get_filepath());
        boards_grid.remove(*cur_child);
    }

    app_window.off_delete_board();
}

ProgressWindow::ProgressWindow()
    : header_bar(),
      add_board_button(),
      root{Gtk::Orientation::VERTICAL},
      board_grid_menu{Gio::Menu::create()},
      board_main_menu{Gio::Menu::create()},
      home_button{},
      menu_button(),
      about_dialog(*this),
      board_grid{},
      board_widget{},
      stack{},
      delete_boards_bar{board_grid, *this} {
    set_title("Progress");

    set_titlebar(header_bar);
    header_bar.pack_end(menu_button);
    header_bar.pack_start(home_button);
    home_button.set_visible(false);
    header_bar.pack_start(add_board_button);

    auto builder =
        Gtk::Builder::create_from_resource("/ui/create-board-dialog.ui");
    auto builder1 =
        Gtk::Builder::create_from_resource("/ui/create-board-dialog.ui");
    create_board_dialog =
        Gtk::Builder::get_widget_derived<ui::CreateBoardDialog>(builder,
                                                                "create-board");
    preferences_board_dialog =
        Gtk::Builder::get_widget_derived<ui::PreferencesBoardDialog>(
            builder1, "create-board", board_widget, *this);

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
    scrl_window.set_size_request(600, 600);
    scrl_window.set_child(board_grid);
    root.append(scrl_window);
    delete_boards_bar.set_halign(Gtk::Align::CENTER);
    delete_boards_bar.set_valign(Gtk::Align::END);
    delete_boards_bar.set_vexpand();
    delete_boards_bar.set_margin_bottom(10);
    root.append(delete_boards_bar);

    stack.add(root, "board_grid");
    stack.add(board_widget, "main_board");
    stack.set_transition_type(Gtk::StackTransitionType::SLIDE_LEFT_RIGHT);

    set_child(stack);
}

ProgressWindow::~ProgressWindow() {
    delete create_board_dialog;
    delete preferences_board_dialog;
}

void ProgressWindow::add_board(std::string board_filepath) {
    auto new_board_card = Gtk::make_managed<BoardCardButton>(board_filepath);
    board_grid.insert(*new_board_card, 0);
    auto fb_child_p = board_grid.get_child_at_index(0);
    new_board_card->signal_clicked().connect(
        [this, new_board_card, fb_child_p]() {
            if (!this->on_delete_mode) {
                Board* board = new Board{new_board_card->get_filepath()};
                stack.set_visible_child("main_board");
                menu_button.set_menu_model(board_main_menu);
                board_widget.set(board, new_board_card);
                preferences_board_dialog->set_board(board);
                home_button.set_visible();
                add_board_button.set_visible(false);
                set_title(board->get_name());
            } else {
                if (fb_child_p->is_selected()) {
                    board_grid.unselect_child(*fb_child_p);
                } else {
                    board_grid.select_child(*fb_child_p);
                }
            }
        });
}   

void ProgressWindow::setup_menu_button() {
    auto action_group = Gio::SimpleActionGroup::create();
    action_group->add_action("about",
                             sigc::mem_fun(*this, &ProgressWindow::show_about));
    action_group->add_action(
        "delete", sigc::mem_fun(*this, &ProgressWindow::on_delete_board));
    action_group->add_action(
        "preferences", [this]() { preferences_board_dialog->set_visible(); });

    board_grid_menu->append("Delete Boards", "win.delete");
    board_grid_menu->append("About", "win.about");

    board_main_menu->append("Preferences", "win.preferences");
    board_main_menu->append("About", "win.about");

    menu_button.insert_action_group("win", action_group);
    menu_button.set_icon_name("open-menu");
    menu_button.set_menu_model(board_grid_menu);
}

void ProgressWindow::show_about() { about_dialog.set_visible(); }

void ProgressWindow::show_create_board() { create_board_dialog->set_visible(); }

void ProgressWindow::go_to_main_menu() {
    stack.set_visible_child("board_grid");
    menu_button.set_menu_model(board_grid_menu);
    home_button.set_visible(false);
    add_board_button.set_visible();
    set_title("Progress");
    board_widget.save();
}

void ProgressWindow::on_delete_board() {
    on_delete_mode = true;
    board_grid.set_selection_mode(Gtk::SelectionMode::MULTIPLE);
    delete_boards_bar.set_reveal_child();
}

void ProgressWindow::off_delete_board() {
    on_delete_mode = false;
    delete_boards_bar.set_reveal_child(false);
    board_grid.set_selection_mode(Gtk::SelectionMode::NONE);
}
}  // namespace ui