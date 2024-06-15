#include "window.h"

#include <glibmm/i18n.h>

#include <filesystem>
#include <format>
#include <iostream>

#include "../core/exceptions.h"
#include "app_info.h"
#include "board-card-button.h"
#include "create_board_dialog.h"
#include "preferences-board-dialog.h"

namespace ui {

ProgressAboutDialog::ProgressAboutDialog(Gtk::Window& parent)
    : parent{parent} {}

ProgressAboutDialog::~ProgressAboutDialog() { g_object_unref(about_dialogp); }

void ProgressAboutDialog::show() {
    setup();
    adw_dialog_present(ADW_DIALOG(about_dialogp), parent.gobj());
}

void ProgressAboutDialog::setup() {
    about_dialogp = adw_about_dialog_new();

    adw_about_dialog_set_application_name(ADW_ABOUT_DIALOG(about_dialogp),
                                          "Progress");
    adw_about_dialog_set_version(
        ADW_ABOUT_DIALOG(about_dialogp),
        std::format("{}.{}.{}", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION).c_str());
    adw_about_dialog_set_developer_name(ADW_ABOUT_DIALOG(about_dialogp),
                                        "Gabriel de Moura");

    adw_about_dialog_set_translator_credits(
        ADW_ABOUT_DIALOG(about_dialogp),
        // Translators: Replace "translator-credits" with your names, one name
        // per line
        _("translator-credits"));

    adw_about_dialog_set_license_type(ADW_ABOUT_DIALOG(about_dialogp),
                                      GTK_LICENSE_MIT_X11);
    adw_about_dialog_set_copyright(ADW_ABOUT_DIALOG(about_dialogp),
                                   _("De Moura © All rights reserved"));
    adw_about_dialog_set_issue_url(
        ADW_ABOUT_DIALOG(about_dialogp),
        "https://github.com/smolBlackCat/progress-tracker/issues");
    adw_about_dialog_set_website(
        ADW_ABOUT_DIALOG(about_dialogp),
        "https://github.com/smolBlackCat/progress-tracker");
    adw_about_dialog_set_application_icon(ADW_ABOUT_DIALOG(about_dialogp),
                                          APPLICATION_ID);
}

DeleteBoardsBar::DeleteBoardsBar(ui::ProgressWindow& app_window)
    : Gtk::Revealer{},
      root{Gtk::Orientation::HORIZONTAL},
      bar_text{_("Select the boards to be deleted")},
      bar_button_delete{_("Delete")},
      bar_button_cancel{_("Cancel")},
      app_window{app_window} {
    set_child(root);
    set_name("delete-board-infobar");
    bar_text.set_name("delete-board-infobar-text");
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
        sigc::mem_fun(app_window, &ProgressWindow::delete_selected_boards));
    bar_button_cancel.signal_clicked().connect(
        sigc::mem_fun(app_window, &ProgressWindow::off_delete_board_mode));
}

ProgressWindow::ProgressWindow(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow{cobject},
      window_builder{builder},
      about_dialog{*this},
      board_widget{*this},
      delete_boards_bar{*this},
      home_button_p{builder->get_widget<Gtk::Button>("home-button")},
      add_board_button_p{builder->get_widget<Gtk::Button>("add-board-button")},
      app_overlay_p{builder->get_widget<Gtk::Overlay>("app-overlay")},
      app_stack_p{builder->get_widget<Gtk::Stack>("app-stack")},
      boards_grid_p{builder->get_widget<Gtk::FlowBox>("boards-grid")},
      board_grid_menu_p{
          window_builder->get_object<Gio::MenuModel>("board-grid-menu")},
      board_menu_p{window_builder->get_object<Gio::MenuModel>("board-menu")},
      app_menu_button_p{
          builder->get_widget<Gtk::MenuButton>("app-menu-button")},
      adw_style_manager{
          adw_style_manager_get_for_display(this->get_display()->gobj())},
      css_provider{Gtk::CssProvider::create()} {
    signal_close_request().connect(
        sigc::mem_fun(*this, &ProgressWindow::on_window_close), true);

    auto cbd_builder = Gtk::Builder::create_from_resource(CREATE_BOARD_DIALOG);
    auto cbd_builder1 = Gtk::Builder::create_from_resource(CREATE_BOARD_DIALOG);
    create_board_dialog =
        Gtk::Builder::get_widget_derived<ui::CreateBoardDialog>(
            cbd_builder, "create-board", *this);
    preferences_board_dialog =
        Gtk::Builder::get_widget_derived<ui::PreferencesBoardDialog>(
            cbd_builder1, "create-board", board_widget);

    create_board_dialog->set_transient_for(*this);
    preferences_board_dialog->set_transient_for(*this);

    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    load_appropriate_style();
    g_signal_connect_after(
        G_OBJECT(adw_style_manager), "notify::dark",
        G_CALLBACK(+[](AdwStyleManager* adw_style_manager, GParamSpec* pspec,
                       gpointer data) {
            reinterpret_cast<ProgressWindow*>(data)->load_appropriate_style();
        }),
        this);

    home_button_p->signal_clicked().connect(
        sigc::mem_fun(*this, &ProgressWindow::on_main_menu));
    add_board_button_p->signal_clicked().connect(
        sigc::mem_fun(*this, &ProgressWindow::show_create_board_dialog));
    setup_menu_button();

    delete_boards_bar.set_halign(Gtk::Align::CENTER);
    delete_boards_bar.set_valign(Gtk::Align::END);
    delete_boards_bar.set_vexpand();
    delete_boards_bar.set_margin_bottom(10);
    app_overlay_p->add_overlay(delete_boards_bar);
    app_stack_p->add(board_widget, "board-page");
}

ProgressWindow::~ProgressWindow() {
    delete create_board_dialog;
    delete preferences_board_dialog;
}

void ProgressWindow::add_board(const std::string& board_filepath) {
    auto board_entry_button =
        Gtk::make_managed<BoardCardButton>(board_filepath);
    boards_grid_p->insert(*board_entry_button, 0);
    auto fb_child_p = boards_grid_p->get_child_at_index(0);
    board_entry_button->signal_clicked().connect(
        [this, board_entry_button, fb_child_p]() {
            if (!this->on_delete_mode) {
                Board* board;
                try {
                    board = new Board{board_entry_button->get_filepath()};
                } catch (std::invalid_argument& err) {
                    Gtk::AlertDialog::create(
                        _("It was not possible to load this board"))
                        ->show(*this);

                    boards_grid_p->remove(*board_entry_button);
                    return;
                }
                on_board_view();
                board_widget.set(board, board_entry_button);
                set_title(board->get_name());
            } else {
                if (fb_child_p->is_selected()) {
                    boards_grid_p->unselect_child(*fb_child_p);
                } else {
                    boards_grid_p->select_child(*fb_child_p);
                }
            }
        });
}

void ProgressWindow::show_about_dialog() { about_dialog.show(); }

void ProgressWindow::show_create_board_dialog() {
    create_board_dialog->open_window();
}

void ProgressWindow::on_delete_board_mode() {
    on_delete_mode = true;
    boards_grid_p->set_selection_mode(Gtk::SelectionMode::MULTIPLE);
    delete_boards_bar.set_reveal_child();
}

void ProgressWindow::off_delete_board_mode() {
    on_delete_mode = false;
    delete_boards_bar.set_reveal_child(false);
    boards_grid_p->set_selection_mode(Gtk::SelectionMode::NONE);
}

void ProgressWindow::on_main_menu() {
    app_stack_p->set_visible_child("board-grid-page");
    app_menu_button_p->set_menu_model(board_grid_menu_p);
    home_button_p->set_visible(false);
    add_board_button_p->set_visible();
    set_title("Progress");
    board_widget.save();
}

void ProgressWindow::on_board_view() {
    app_stack_p->set_visible_child("board-page");
    app_menu_button_p->set_menu_model(board_menu_p);
    home_button_p->set_visible();
    add_board_button_p->set_visible(false);
}

void ProgressWindow::delete_selected_boards() {
    auto selected_children = boards_grid_p->get_selected_children();
    for (auto& child : selected_children) {
        ui::BoardCardButton* cur_child =
            (ui::BoardCardButton*)child->get_child();
        std::filesystem::remove(cur_child->get_filepath());
        boards_grid_p->remove(*cur_child);
    }

    off_delete_board_mode();
}

void ProgressWindow::setup_menu_button() {
    auto action_group = Gio::SimpleActionGroup::create();
    action_group->add_action(
        "about", sigc::mem_fun(*this, &ProgressWindow::show_about_dialog));
    action_group->add_action(
        "delete", sigc::mem_fun(*this, &ProgressWindow::on_delete_board_mode));
    action_group->add_action(
        "preferences", sigc::mem_fun(*preferences_board_dialog,
                                     &PreferencesBoardDialog::open_window));

    app_menu_button_p->insert_action_group("win", action_group);
}

void ProgressWindow::load_appropriate_style() {
    if (adw_style_manager_get_dark(adw_style_manager)) {
        css_provider->load_from_resource(ProgressWindow::STYLE_DARK_CSS);
    } else {
        css_provider->load_from_resource(ProgressWindow::STYLE_CSS);
    }
}

bool ProgressWindow::on_window_close() {
    ;
    if (app_stack_p->get_visible_child_name() == "board-page") {
        board_widget.save();
    }
    set_visible(false);
    return true;
}
}  // namespace ui
