#include <app_info.h>
#include <core/exceptions.h>
#include <dialog/create_board_dialog.h>
#include <dialog/preferences-board-dialog.h>
#include <glibmm/i18n.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <widgets/card-widget.h>

#include <format>
#include <thread>

#include "window.h"

namespace ui {

ProgressWindow::ProgressWindow(BaseObjectType* cobject,
                               const Glib::RefPtr<Gtk::Builder>& b,
                               Glib::RefPtr<Gio::Settings>& progress_settings,
                               BoardManager& manager)
    : Gtk::ApplicationWindow{cobject},
      m_manager{manager},
      board_widget{manager},
      home_button_p{b->get_widget<Gtk::Button>("home-button")},
      add_board_button_p{b->get_widget<Gtk::Button>("add-board-button")},
      board_delete_button{b->get_widget<Gtk::Button>("delete-button")},
      cancel_delete_button{b->get_widget<Gtk::Button>("cancel-delete-button")},
      app_overlay_p{b->get_widget<Gtk::Overlay>("app-overlay")},
      app_stack_p{b->get_widget<Gtk::Stack>("app-stack")},
      boards_grid_p{b->get_widget<Gtk::FlowBox>("boards-grid")},
      board_grid_menu_p{b->get_object<Gio::MenuModel>("board-grid-menu")},
      board_menu_p{b->get_object<Gio::MenuModel>("board-menu")},
      app_menu_button_p{b->get_widget<Gtk::MenuButton>("app-menu-button")},
      action_bar_p{b->get_widget<Gtk::ActionBar>("action-bar")},
      adw_style_manager{
          adw_style_manager_get_for_display(this->get_display()->gobj())},
      css_provider{Gtk::CssProvider::create()},
      progress_settings{progress_settings},
      card_dialog{},
      sh_window{b->get_widget<Gtk::ShortcutsWindow>("progress-shortcuts")} {
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

    // We're not overriding 'on_close_request' because the behaviour is not
    // executed. This may be due to the fact that cobject is actually a
    // AdwApplicationWindow, not a GtkApplicationWindow
    this->signal_close_request().connect(
        sigc::mem_fun(*this, &ProgressWindow::on_close), true);
    boards_grid_p->signal_selected_children_changed().connect(
        sigc::track_object(
            [this]() {
                const int size =
                    this->boards_grid_p->get_selected_children().size();
                if (size > 0) {
                    std::string selected_text = ngettext(
                        "{} board selected", "{} boards selected", size);
                    set_title(std::vformat(selected_text,
                                           std::make_format_args(size)));
                } else {
                    set_title(_("No board has been selected yet"));
                }
            },
            *this));

    board_delete_button->signal_clicked().connect(
        sigc::mem_fun(*this, &ProgressWindow::delete_selected_boards));
    cancel_delete_button->signal_clicked().connect(
        sigc::mem_fun(*this, &ProgressWindow::off_delete_board_mode));

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

    using WindowShortcut =
        std::pair<const char*,
                  std::function<bool(Gtk::Widget&, const Glib::VariantBase&)>>;

    const std::array<WindowShortcut, 5> shortcuts = {
        WindowShortcut{"<Control>D",
                       [this](Gtk::Widget&, const Glib::VariantBase&) {
                           if (app_stack_p->get_visible_child_name() ==
                               "board-grid-page") {
                               this->on_delete_board_mode();
                           }

                           return true;
                       }},
        WindowShortcut{"<Control>B",
                       [this](Gtk::Widget&, const Glib::VariantBase&) {
                           if (app_stack_p->get_visible_child_name() ==
                               "board-grid-page") {
                               auto create_board_dialog =
                                   CreateBoardDialog::create(m_manager);
                               create_board_dialog->open(*this);
                           }

                           return true;
                       }},
        WindowShortcut{
            "<Control>P",
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                if (app_stack_p->get_visible_child_name() == "board-page") {
                    auto preference_dialog =
                        PreferencesBoardDialog::create(this->board_widget);
                    preference_dialog->open(*this);
                }
                return true;
            }},
        WindowShortcut{"<Control>H",
                       [this](Gtk::Widget&, const Glib::VariantBase&) {
                           if (app_stack_p->get_visible_child_name() ==
                               "board-page") {
                               this->on_main_menu();
                           }
                           return true;
                       }},
        WindowShortcut{"<Control>W",
                       [this](Gtk::Widget&, const Glib::VariantBase&) {
                           close();
                           return true;
                       }}};

    auto shortcut_controller = Gtk::ShortcutController::create();
    for (const auto& [keybind, callback] : shortcuts) {
        shortcut_controller->add_shortcut(
            Gtk::Shortcut::create(Gtk::ShortcutTrigger::parse_string(keybind),
                                  Gtk::CallbackAction::create(callback)));
    }
    add_controller(shortcut_controller);

#if defined(DEVELOPMENT)
    add_css_class("devel");
#endif

    home_button_p->signal_clicked().connect(
        sigc::mem_fun(*this, &ProgressWindow::on_main_menu));
    add_board_button_p->signal_clicked().connect([this]() {
        auto create_board_dialog = CreateBoardDialog::create(m_manager);

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

    m_manager.signal_add_board().connect(
        sigc::mem_fun(*this, &ProgressWindow::add_board_handler));
    m_manager.signal_remove_board().connect(
        sigc::mem_fun(*this, &ProgressWindow::remove_board_handler));
    m_manager.signal_save_board().connect(
        sigc::mem_fun(*this, &ProgressWindow::save_board_handler));

    sh_window->set_application(this->get_application());

    app_stack_p->add(board_widget, "board-page");
}

ProgressWindow::~ProgressWindow() {}

void ProgressWindow::add_board_handler(LocalBoard board_entry) {
    add_local_board_entry(board_entry);
    boards_grid_p->invalidate_sort();
}

// FIXME: We could actually keep track of the boardcardbuttons added instead of
// searching linearly. This may be optmised to an amortised O(1)
void ProgressWindow::remove_board_handler(LocalBoard board_entry) {
    for (Widget* fb_child : boards_grid_p->get_children()) {
        BoardCardButton* cur = static_cast<BoardCardButton*>(
            static_cast<Gtk::FlowBoxChild*>(fb_child)->get_child());

        if (cur->get_board() == board_entry.board) {
            boards_grid_p->remove(*cur);
        }
    }
}

void ProgressWindow::save_board_handler(LocalBoard board) {
    boards_grid_p->invalidate_sort();
}

void ProgressWindow::add_local_board_entry(LocalBoard board_entry) {
    auto board_card_button = Gtk::make_managed<BoardCardButton>(board_entry);
    auto fb_child_p = Gtk::make_managed<Gtk::FlowBoxChild>();
    fb_child_p->set_child(*board_card_button);
    fb_child_p->set_focusable(false);
    boards_grid_p->append(*fb_child_p);
    board_card_button->signal_clicked().connect([this, board_entry,
                                                 board_card_button,
                                                 fb_child_p]() {
        if (!this->on_delete_mode) {
            app_stack_p->set_visible_child("loading-page",
                                           Gtk::StackTransitionType::CROSSFADE);
            add_board_button_p->set_sensitive(false);
            app_menu_button_p->set_sensitive(false);

            std::thread{[this, board_entry, &board_card_button]() {
                spdlog::get("app")->debug(
                    "Starting helper thread to load board");
                try {
                    this->cur_board_entry = board_card_button;
                    this->cur_board =
                        this->m_manager.local_open(board_entry.filename);
                } catch (std::invalid_argument& err) {
                    this->cur_board = nullptr;
                    spdlog::get("app")->error("Failed to load board: {}",
                                              err.what());
                }
                this->dispatcher.emit();
                spdlog::get("app")->debug(
                    "Helper thread finished. Dispatching for main GUI thread");
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
    action_bar_p->set_revealed();

    set_title(_("No board has been selected yet"));

    // TAB clicks won't include them, making more manageable to delete boards
    add_board_button_p->set_visible(false);
    app_menu_button_p->set_visible(false);

    spdlog::get("app")->info(
        "[Progress Window] User entered delete board mode");
}

void ProgressWindow::off_delete_board_mode() {
    on_delete_mode = false;
    action_bar_p->set_revealed(false);
    boards_grid_p->set_selection_mode(Gtk::SelectionMode::NONE);

    add_board_button_p->set_visible();
    app_menu_button_p->set_visible();

    set_title("Progress");

    spdlog::get("app")->info(
        "[Progress Window] User has left delete board mode");
}

void ProgressWindow::on_main_menu() {
    app_stack_p->set_visible_child("board-grid-page");
    sh_window->property_view_name().set_value("board-grid-view");
    app_menu_button_p->set_menu_model(board_grid_menu_p);
    home_button_p->set_visible(false);
    add_board_button_p->set_visible();
    set_title("Progress");
    if (cur_board && cur_board_entry) board_widget.save();

    spdlog::get("app")->info(
        "[Progress Window] Current view changed to board grid");
}

void ProgressWindow::on_board_view() {
    app_stack_p->set_visible_child("board-page");
    sh_window->property_view_name().set_value("board-view");
    app_menu_button_p->set_menu_model(board_menu_p);
    home_button_p->set_visible();
    add_board_button_p->set_visible(false);

    spdlog::get("app")->info(
        "[Progress Window] App view changed to board view");
}

// FIXME: This super inefficient because we're running at almost O(n²)
void ProgressWindow::delete_selected_boards() {
    auto selected_children = boards_grid_p->get_selected_children();
    for (auto& child : selected_children) {
        ui::BoardCardButton* cur_child =
            (ui::BoardCardButton*)child->get_child();
        m_manager.local_remove(cur_child->get_board());
    }

    spdlog::get("app")->info("[Progress Window] User has deleted {} boards",
                             selected_children.size());

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
    spdlog::get("app")->info("[Progress Window] Show about dialog");
}

void ProgressWindow::show_card_dialog(CardWidget* card_widget) {
    card_dialog.open(*this, card_widget);
}

void ProgressWindow::show_shortcuts_dialog() {
    sh_window->set_visible();

    spdlog::get("app")->info("[Progress Window] Shortcuts dialog opened");
}

void ProgressWindow::setup_menu_button() {
    auto action_group = Gio::SimpleActionGroup::create();
    action_group->add_action(
        "about", sigc::mem_fun(*this, &ProgressWindow::show_about_dialog));
    action_group->add_action(
        "shortcuts",
        sigc::mem_fun(*this, &ProgressWindow::show_shortcuts_dialog));
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
        spdlog::get("ui")->debug("[Progress Window] Loaded dark style");
    } else {
        spdlog::get("ui")->debug("[Progress Window] Loaded light style");
        css_provider->load_from_resource(ProgressWindow::STYLE_CSS);
    }
}

void ProgressWindow::on_board_loading_done() {
    if (cur_board) {
        on_board_view();
        board_widget.set(cur_board);
        set_title(cur_board->get_name());
        spdlog::get("ui")->info(
            "[Progress Window] Board view loaded successfully");
    } else {
        // Loading thread has failed. Warn user and log
        Gtk::AlertDialog::create(_("It was not possible to load this board"))
            ->show(*this);
        boards_grid_p->remove(*cur_board_entry);
        cur_board_entry = nullptr;
        on_main_menu();

        spdlog::get("ui")->error(
            "[Progress Window] Failed to load board widget");
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

    spdlog::get("app")->info("Application window has been closed");
    return true;
}
}  // namespace ui

