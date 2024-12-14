#include "window.h"

#include <app_info.h>
#include <core/exceptions.h>
#include <dialog/create_board_dialog.h>
#include <dialog/preferences-board-dialog.h>
#include <glibmm/i18n.h>
#include <widgets/board-card-button.h>

#include <filesystem>
#include <format>
#include <thread>

namespace ui {

DeleteBoardsBar::DeleteBoardsBar(ui::ProgressWindow& app_window)
    : Gtk::Revealer{},
      root{Gtk::Orientation::HORIZONTAL},
      bar_text{_("Select the boards to be deleted")},
      bar_button_delete{_("Delete")},
      bar_button_cancel{_("Cancel")} {
    set_child(root);
    add_css_class("delete-board-infobar");
    bar_text.add_css_class("delete-board-infobar-text");
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
                               const Glib::RefPtr<Gtk::Builder>& b,
                               Glib::RefPtr<Gio::Settings>& progress_settings)
    : Gtk::ApplicationWindow{cobject},
      board_widget{},
      delete_boards_bar{*this},
      home_button_p{b->get_widget<Gtk::Button>("home-button")},
      add_board_button_p{b->get_widget<Gtk::Button>("add-board-button")},
      app_overlay_p{b->get_widget<Gtk::Overlay>("app-overlay")},
      app_stack_p{b->get_widget<Gtk::Stack>("app-stack")},
      boards_grid_p{b->get_widget<Gtk::FlowBox>("boards-grid")},
      board_grid_menu_p{b->get_object<Gio::MenuModel>("board-grid-menu")},
      board_menu_p{b->get_object<Gio::MenuModel>("board-menu")},
      app_menu_button_p{b->get_widget<Gtk::MenuButton>("app-menu-button")},
      adw_style_manager{
          adw_style_manager_get_for_display(this->get_display()->gobj())},
      css_provider{Gtk::CssProvider::create()},
      progress_settings{progress_settings} {
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

    // We're not overriding 'on_close_request' because the behaviour is not
    // executed. This may be due to the fact that cobject is actually a
    // AdwApplicationWindow, not a GtkApplicationWindow
    this->signal_close_request().connect(
        sigc::mem_fun(*this, &ProgressWindow::on_close), true);

    dispatcher.connect(
        sigc::mem_fun(*this, &ProgressWindow::on_board_loading_done));

    load_appropriate_style();

    // FIXME: Use automatic stylesheet loading provided by adwaita
    g_signal_connect_after(
        adw_style_manager, "notify::dark",
        G_CALLBACK(+[](AdwStyleManager* adw_style_manager, GParamSpec* pspec,
                       gpointer data) {
            reinterpret_cast<ProgressWindow*>(data)->load_appropriate_style();
        }),
        this);

#if defined(DEVELOPMENT)
    add_css_class("devel");
#endif

    home_button_p->signal_clicked().connect(
        sigc::mem_fun(*this, &ProgressWindow::on_main_menu));
    add_board_button_p->signal_clicked().connect([this]() {
        auto create_board_dialog = CreateBoardDialog::create(*this);

        create_board_dialog->open(*this);
    });
    setup_menu_button();

    boards_grid_p->set_sort_func([](Gtk::FlowBoxChild* child1,
                                    Gtk::FlowBoxChild* child2) {
        ui::BoardCardButton* bcb1 = (ui::BoardCardButton*)child1->get_child();
        ui::BoardCardButton* bcb2 = (ui::BoardCardButton*)child2->get_child();

        if (*bcb1 > *bcb2) {
            return -1;
        } else if (*bcb1 < *bcb2) {
            return 1;
        } else {
            return 0;
        }
    });

    delete_boards_bar.set_halign(Gtk::Align::CENTER);
    delete_boards_bar.set_valign(Gtk::Align::END);
    delete_boards_bar.set_vexpand();
    delete_boards_bar.set_margin_bottom(10);
    app_overlay_p->add_overlay(delete_boards_bar);
    app_stack_p->add(board_widget, "board-page");
}

ProgressWindow::~ProgressWindow() {}

void ProgressWindow::add_local_board(BoardBackend board_backend) {
    auto board_card_button = Gtk::make_managed<BoardCardButton>(board_backend);
    auto fb_child_p = Gtk::make_managed<Gtk::FlowBoxChild>();
    fb_child_p->set_child(*board_card_button);
    boards_grid_p->append(*fb_child_p);
    board_card_button->signal_clicked().connect(
        [this, board_card_button, fb_child_p]() {
            if (!this->on_delete_mode) {
                app_stack_p->set_visible_child(
                    "loading-page", Gtk::StackTransitionType::CROSSFADE);
                add_board_button_p->set_sensitive(false);
                app_menu_button_p->set_sensitive(false);

                std::thread{[this, board_card_button]() {
                    try {
                        this->cur_board_entry = board_card_button;
                        this->cur_board = std::make_shared<Board>(
                            board_card_button->get_backend().load());
                    } catch (std::invalid_argument& err) {
                        this->cur_board = nullptr;
                    }
                    this->dispatcher.emit();
                }}.detach();
            } else {
                if (fb_child_p->is_selected()) {
                    boards_grid_p->unselect_child(*fb_child_p);
                } else {
                    boards_grid_p->select_child(*fb_child_p);
                }
            }
        });
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
    if (cur_board && cur_board_entry) board_widget.save();
    boards_grid_p->invalidate_sort();
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
        std::filesystem::remove(
            cur_child->get_backend().get_attribute("filepath"));
        boards_grid_p->remove(*cur_child);
    }

    off_delete_board_mode();
}

void ProgressWindow::show_about_dialog() {
    adw_show_about_dialog(
        GTK_WIDGET(this->gobj()), "application-name", "Progress",
        "application-icon", APPLICATION_ID, "version",
        std::format("{}.{}.{}", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION)
            .c_str(),
        "copyright", _("De Moura © All rights reserved"), "license-type",
        GTK_LICENSE_MIT_X11, "developer-name", "Gabriel de Moura",
        "translator-credits", _("translator-credits"), "issue-url",
        "https://github.com/smolBlackCat/progress-tracker/issues", "website",
        "https://github.com/smolBlackCat/progress-tracker", NULL);
}

void ProgressWindow::setup_menu_button() {
    auto action_group = Gio::SimpleActionGroup::create();
    action_group->add_action(
        "about", sigc::mem_fun(*this, &ProgressWindow::show_about_dialog));
    action_group->add_action(
        "delete", sigc::mem_fun(*this, &ProgressWindow::on_delete_board_mode));
    action_group->add_action("preferences", [this]() {
        auto preference_dialog =
            PreferencesBoardDialog::create(this->board_widget);
        preference_dialog->open(*this);
    });

    app_menu_button_p->insert_action_group("win", action_group);
}

void ProgressWindow::load_appropriate_style() {
    if (adw_style_manager_get_dark(adw_style_manager)) {
        css_provider->load_from_resource(ProgressWindow::STYLE_DARK_CSS);
    } else {
        css_provider->load_from_resource(ProgressWindow::STYLE_CSS);
    }
}

void ProgressWindow::on_board_loading_done() {
    if (cur_board) {
        on_board_view();
        board_widget.set(cur_board, cur_board_entry);
        set_title(cur_board->get_name());
    } else {
        // cur_board and cur_board_entry are still nullptrs because the loading
        // thread has failed, therefore, go back to the main menu
        Gtk::AlertDialog::create(_("It was not possible to load this board"))
            ->show(*this);
        boards_grid_p->remove(*cur_board_entry);
        cur_board_entry = nullptr;
        on_main_menu();
    }
    add_board_button_p->set_sensitive();
    app_menu_button_p->set_sensitive();
}

bool ProgressWindow::on_close() {
    if (app_stack_p->get_visible_child_name() == "board-page") {
        board_widget.save();
    }

    progress_settings->set_boolean("window-maximized", is_maximized());
    progress_settings->set_int("window-height", get_height());
    progress_settings->set_int("window-width", get_width());
    set_visible(false);
    return true;
}
}  // namespace ui
