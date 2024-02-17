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

DeleteBoardsBar::DeleteBoardsBar(Gtk::FlowBox* boards_grid,
                                 ui::ProgressWindow& app_window)
    : Gtk::Revealer{},
      root{Gtk::Orientation::HORIZONTAL},
      bar_text{_("Select the boards to be deleted")},
      bar_button_delete{_("Delete")},
      bar_button_cancel{_("Cancel")},
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
    auto selected_children = boards_grid->get_selected_children();
    for (auto& fb_child_p : selected_children) {
        ui::BoardCardButton* cur_child =
            (ui::BoardCardButton*)fb_child_p->get_child();
        std::filesystem::remove(cur_child->get_filepath());
        boards_grid->remove(*cur_child);
    }

    app_window.off_delete_board();
}

ProgressWindow::ProgressWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window{cobject},
      window_builder{builder},
      about_dialog{*this},
      board_widget{},
      delete_boards_bar{builder->get_widget<Gtk::FlowBox>("boards-grid"), *this} {

    auto cbd_builder =
        Gtk::Builder::create_from_resource("/ui/create-board-dialog.ui");
    auto cbd_builder1 =
        Gtk::Builder::create_from_resource("/ui/create-board-dialog.ui");
    create_board_dialog =
        Gtk::Builder::get_widget_derived<ui::CreateBoardDialog>(cbd_builder,
                                                                "create-board");
    preferences_board_dialog =
        Gtk::Builder::get_widget_derived<ui::PreferencesBoardDialog>(
            cbd_builder1, "create-board", board_widget, *this);

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
        .connect(sigc::mem_fun(*this, &ProgressWindow::go_to_main_menu));
    builder->get_widget<Gtk::Button>("add-board-button")
        ->signal_clicked()
        .connect(sigc::mem_fun(*this, &ProgressWindow::show_create_board));
    setup_menu_button();

    delete_boards_bar.set_halign(Gtk::Align::CENTER);
    delete_boards_bar.set_valign(Gtk::Align::END);
    delete_boards_bar.set_vexpand();
    delete_boards_bar.set_margin_bottom(10);
    builder->get_widget<Gtk::Box>("root-box")->append(delete_boards_bar);
    builder->get_widget<Gtk::Stack>("app-stack")->add(board_widget, "board-page");
}

ProgressWindow::~ProgressWindow() {
    delete create_board_dialog;
    delete preferences_board_dialog;
}

void ProgressWindow::add_board(std::string board_filepath) {
    auto new_board_card = Gtk::make_managed<BoardCardButton>(board_filepath);
    window_builder->get_widget<Gtk::FlowBox>("boards-grid")->insert(*new_board_card, 0);
    auto fb_child_p = window_builder->get_widget<Gtk::FlowBox>("boards-grid")
        ->get_child_at_index(0);
    new_board_card->signal_clicked().connect(
        [this, new_board_card, fb_child_p]() {
            if (!this->on_delete_mode) {
                Board* board = new Board{new_board_card->get_filepath()};
                window_builder->get_widget<Gtk::Stack>("app-stack")
                    ->set_visible_child("board-page");
                window_builder->get_widget<Gtk::MenuButton>("app-menu-button")
                    ->set_menu_model(
                        window_builder
                            ->get_object<Gio::MenuModel>("board-menu"));
                board_widget.set(board, new_board_card);
                preferences_board_dialog->set_board(board);
                window_builder->get_widget<Gtk::Button>("home-button")->set_visible();
                window_builder->get_widget<Gtk::Button>("add-board-button")->set_visible(false);
                set_title(board->get_name());
            } else {
                if (fb_child_p->is_selected()) {
                    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
                        ->unselect_child(*fb_child_p);
                } else {
                    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
                        ->select_child(*fb_child_p);
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

    window_builder->get_widget<Gtk::MenuButton>("app-menu-button")
        ->insert_action_group("win", action_group);
}

void ProgressWindow::show_about() { about_dialog.set_visible(); }

void ProgressWindow::show_create_board() { create_board_dialog->set_visible(); }

void ProgressWindow::go_to_main_menu() {
    window_builder->get_widget<Gtk::Stack>("app-stack")
        ->set_visible_child("board-grid-page");
    window_builder->get_widget<Gtk::MenuButton>("app-menu-button")
                    ->set_menu_model(
                        window_builder
                            ->get_object<Gio::MenuModel>("board-grid-menu"));
    window_builder->get_widget<Gtk::Button>("home-button")->set_visible(false);
    window_builder->get_widget<Gtk::Button>("add-board-button")->set_visible();
    set_title("Progress");
    board_widget.save();
}

void ProgressWindow::on_delete_board() {
    on_delete_mode = true;
    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
        ->set_selection_mode(Gtk::SelectionMode::MULTIPLE);
    delete_boards_bar.set_reveal_child();
}

void ProgressWindow::off_delete_board() {
    on_delete_mode = false;
    delete_boards_bar.set_reveal_child(false);
    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
        ->set_selection_mode(Gtk::SelectionMode::NONE);
}
}  // namespace ui