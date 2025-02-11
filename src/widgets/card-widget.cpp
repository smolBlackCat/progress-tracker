#include "card-widget.h"

#include <dialog/card-dialog.h>
#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <utils.h>

#include <chrono>
#include <numeric>

#include "cardlist-widget.h"
#include "gtk/gtk.h"
#include "window.h"

extern "C" {
static void card_class_init(void* g_class, void* data) {
    g_return_if_fail(GTK_IS_WIDGET_CLASS(g_class));
    gtk_widget_class_set_css_name(GTK_WIDGET_CLASS(g_class), "card");
}

static void card_init(GTypeInstance* instance, void* g_class) {
    g_return_if_fail(GTK_IS_WIDGET(instance));

    gtk_widget_set_focusable(GTK_WIDGET(instance), TRUE);
    gtk_widget_set_receives_default(GTK_WIDGET(instance), TRUE);
}
}

namespace ui {
CardInit::CardInit()
    : Glib::ExtraClassInit(card_class_init, nullptr, card_init) {}

CardWidget::CardWidget(std::shared_ptr<Card> card, bool is_new)
    : Glib::ObjectBase{"CardWidget"},
      CardInit{},
      BaseItem{Gtk::Orientation::VERTICAL, 0},
      card{card},
      cardlist_p{nullptr},
      is_new{is_new},
      root_box{Gtk::Orientation::VERTICAL},
      card_cover_revealer{},
      card_entry_revealer{},
      card_cover_picture{},
      card_label{},
      complete_tasks_label{},
      due_date_label{},
      card_entry{},
      card_menu_button{},
      popover_menu{},
      key_controller{Gtk::EventControllerKey::create()},
      card_label_click_controller{Gtk::GestureClick::create()},
      click_controller{Gtk::GestureClick::create()},
      card_menu_model{Gio::Menu::create()},
      color_dialog{Gtk::ColorDialog::create()} {
    // Setup Widgets
    root_box.set_spacing(4);
    root_box.set_size_request(240, -1);

    // Card's cover
    root_box.append(card_cover_revealer);
    card_cover_revealer.set_child(card_cover_picture);
    card_cover_picture.set_content_fit(Gtk::ContentFit::COVER);
    card_cover_picture.add_css_class("card");
    card_cover_picture.set_size_request(-1, 50);

    // Card Body
    Gtk::Box& card_body = *Gtk::make_managed<Gtk::Box>();
    card_body.set_margin(4);
    card_body.set_spacing(10);
    root_box.append(card_body);

    // inner box
    Gtk::Box& card_data_box =
        *Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    card_data_box.set_spacing(4);
    card_data_box.set_valign(Gtk::Align::CENTER);

    Gtk::Box& card_label_box =
        *Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    card_label_box.append(card_label);
    card_label.set_halign(Gtk::Align::START);
    card_label.set_hexpand();
    card_label.set_label(Glib::locale_to_utf8(_("New Card")));
    card_label.set_wrap();
    card_label.set_natural_wrap_mode(Gtk::NaturalWrapMode::WORD);
    card_label.set_wrap_mode(Pango::WrapMode::WORD_CHAR);

    card_label_box.append(card_entry_revealer);
    card_entry_revealer.set_child(card_entry);
    card_entry.set_hexpand();
    card_data_box.append(card_label_box);

    Gtk::Box& card_info_box = *Gtk::make_managed<Gtk::Box>();
    card_info_box.set_spacing(15);

    card_info_box.append(complete_tasks_label);
    complete_tasks_label.set_visible(false);

    card_info_box.append(due_date_label);
    due_date_label.add_css_class("due-date");
    due_date_label.set_visible(false);

    card_data_box.append(card_info_box);

    card_body.append(card_data_box);

    card_menu_button.set_halign(Gtk::Align::CENTER);
    card_menu_button.set_has_frame(false);
    card_menu_button.set_icon_name("view-more-horizontal-symbolic");
    card_menu_button.set_menu_model(card_menu_model);
    card_menu_button.set_can_focus(false);

    card_menu_model->append(_("Rename"), "card.rename");
    card_menu_model->append(_("Card Details"), "card.details");
    auto card_cover_submenu = Gio::Menu::create();
    card_cover_submenu->append(_("Set Color"), "card.set-color");
    card_cover_submenu->append(_("Unset Color"), "card.unset-color");
    card_menu_model->append_submenu(_("Card Cover"), card_cover_submenu);
    card_menu_model->append(_("Remove"), "card.remove");

    card_menu_button.set_tooltip_text(_("Card Options"));
    card_menu_button.set_valign(Gtk::Align::CENTER);
    card_body.append(card_menu_button);

    root_box.insert_at_end(*this);

    // CardWidget Setup

    set_title(card->get_name());

    if (is_new) {
        on_rename();  // Open card on rename mode by default whenever a new card
                      // is created
    }

    auto card_actions = Gio::SimpleActionGroup::create();
    card_actions->add_action("rename",
                             sigc::mem_fun(*this, &CardWidget::on_rename));
    card_actions->add_action(
        "details", sigc::mem_fun(*this, &CardWidget::open_card_details_dialog));
    card_actions->add_action(
        "set-color", sigc::mem_fun(*this, &CardWidget::open_color_dialog));
    card_actions->add_action("unset-color",
                             sigc::mem_fun(*this, &CardWidget::clear_color));
    card_actions->add_action(
        "remove", sigc::mem_fun(*this, &CardWidget::remove_from_parent));
    card_menu_button.insert_action_group("card", card_actions);

    card_cover_revealer.property_child_revealed().signal_changed().connect(
        [this]() {
            if (!card_cover_revealer.get_child_revealed())
                this->card_cover_picture.set_paintable(nullptr);
        });

    auto shortcut_controller = Gtk::ShortcutController::create();
    shortcut_controller->set_scope(Gtk::ShortcutScope::LOCAL);
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>N"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                auto new_card = cardlist_p->add(Card{_("New Card")}, true);
                cardlist_p->reorder(*new_card, *this);

                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>D"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                this->open_card_details_dialog();
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>R"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                this->on_rename();
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control><Shift>C"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                this->open_color_dialog();
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Delete"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                this->remove_from_parent();
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Up"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                CardWidget* previous_card =
                    static_cast<CardWidget*>(this->get_prev_sibling());
                if (previous_card) {
                    this->cardlist_p->reorder(*previous_card, *this);
                }
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Down"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                Widget* next = this->get_next_sibling();

                if (!(G_TYPE_CHECK_INSTANCE_TYPE(next->gobj(),
                                                 Gtk::Button::get_type()))) {
                    this->cardlist_p->reorder(*this,
                                              *static_cast<CardWidget*>(next));
                }
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Left"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                CardlistWidget* prev_parent = static_cast<CardlistWidget*>(
                    this->cardlist_p->get_prev_sibling());

                if (prev_parent) {
                    this->remove_from_parent();
                    auto this_card = prev_parent->add(*this->card);
                    this_card->grab_focus();
                }

                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Right"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                Widget* next_parent = this->cardlist_p->get_next_sibling();

                if (!(G_TYPE_CHECK_INSTANCE_TYPE(next_parent->gobj(),
                                                 Gtk::Button::get_type()))) {
                    CardlistWidget* next_cardlist =
                        static_cast<CardlistWidget*>(next_parent);
                    this->remove_from_parent();
                    auto this_card = next_cardlist->add(*this->card);
                    this_card->grab_focus();
                }

                return true;
            })));
    this->add_controller(shortcut_controller);

    popover_menu.set_parent(root_box);
    popover_menu.set_menu_model(card_menu_model);
    popover_menu.insert_action_group("card", card_actions);

    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (card_entry_revealer.get_child_revealed()) {
                switch (keyval) {
                    case GDK_KEY_Return: {
                        this->on_confirm_changes();
                        this->off_rename();
                        break;
                    }
                    case GDK_KEY_Escape: {
                        this->on_cancel_changes();
                        this->off_rename();
                        break;
                    }
                }
            }
        });

    card_label_click_controller->signal_released().connect(
        [this](int n_pressed, double x, double y) {
            if (n_pressed >= 1 && !card_entry_revealer.get_child_revealed() &&
                this->card_label_click_controller->get_current_button() ==
                    GDK_BUTTON_PRIMARY) {
                this->on_rename();
            }
        });

    click_controller->set_button(GDK_BUTTON_SECONDARY);
    click_controller->signal_released().connect(
        [this](int n_pressed, double x, double y) {
            auto clicked = this->click_controller->get_current_button();
            if (clicked == GDK_BUTTON_SECONDARY && n_pressed >= 1) {
                this->popover_menu.set_pointing_to(Gdk::Rectangle(x, y, 0, 0));
                this->popover_menu.popup();
            }
        });

    auto drop_motion_controller = Gtk::DropControllerMotion::create();
    drop_motion_controller->signal_motion().connect(
        [this](double x, double y) { this->add_css_class("card-to-drop"); });
    drop_motion_controller->signal_leave().connect(
        [this]() { this->remove_css_class("card-to-drop"); });

    card_entry.add_controller(key_controller);
    card_label.add_controller(card_label_click_controller);
    add_controller(click_controller);
    add_controller(drop_motion_controller);

    if (card->is_color_set()) {
        Color card_color = card->get_color();
        _set_color(Gdk::RGBA{static_cast<float>(std::get<0>(card_color)) / 255,
                             static_cast<float>(std::get<1>(card_color)) / 255,
                             static_cast<float>(std::get<2>(card_color)) / 255,
                             std::get<3>(card_color)});

        if (!is_new) {
            card->set_modified(false);
        }
    }
    if (card->get_due_date().ok()) {
        update_due_date();
    }

    if (!card->get_tasks().empty()) {
        update_complete_tasks();
    }

    set_tooltip_text(create_details_text());
    setup_drag_and_drop();
}

void CardWidget::set_title(const std::string& label) {
    card_label.set_label(label);
    card_entry.set_text(label);
}

std::string CardWidget::get_title() const { return card_label.get_label(); }

void CardWidget::remove_from_parent() {
    if (cardlist_p) {
        cardlist_p->remove(this);
    }
}

void CardWidget::set_cardlist(CardlistWidget* cardlist_p) {
    if (cardlist_p) {
        if (this->cardlist_p) {
            spdlog::get("ui")->debug(
                "CardWidget \"{}\" changed parent from CardlistWidget \"{}\" "
                "to "
                "CardlistWidget \"{}\"",
                card->get_name(), this->cardlist_p->get_cardlist()->get_name(),
                cardlist_p->get_cardlist()->get_name());
        }
        this->cardlist_p = cardlist_p;
    }
}

std::shared_ptr<Card> CardWidget::get_card() { return card; }

CardlistWidget const* CardWidget::get_cardlist_widget() const {
    return cardlist_p;
}

void CardWidget::update_complete_tasks() {
    if (card->get_tasks().empty()) {
        complete_tasks_label.set_label("");
        complete_tasks_label.set_visible(false);
    } else {
        complete_tasks_label.set_visible();
        float n_complete_tasks =
            std::accumulate(card->get_tasks().begin(), card->get_tasks().end(),
                            0, [](int acc, const std::shared_ptr<Task>& task) {
                                return task->get_done() ? ++acc : acc;
                            });
        complete_tasks_label.set_label(
            std::format("{}/{}", n_complete_tasks, card->get_tasks().size()));

        // Silences warning about deleting empty css classes
        update_complete_tasks_style(n_complete_tasks);
    }

    spdlog::get("ui")->debug(
        "CardWidget \"{}\" has updated complete tasks label", card->get_name());
}

void CardWidget::update_due_date() {
    if (card->get_due_date().ok()) {
        due_date_label.set_visible();

        auto sys_days = std::chrono::sys_days(card->get_due_date());
        std::time_t time = std::chrono::system_clock::to_time_t(sys_days);

        char date_str[100];
        std::strftime(date_str, sizeof(date_str), "%d %b, %Y",
                      std::gmtime(&time));

        due_date_label.set_label(_("Due: ") + Glib::ustring{date_str});

        update_due_date_label_style();
    } else {
        due_date_label.set_visible(false);
    }

    spdlog::get("ui")->debug("CardWidget \"{}\" has updated due date label",
                             card->get_name());
}

void CardWidget::update_due_date_label_style() {
    // We will only update the due date label style if the card has a due date
    if (card->get_due_date().ok()) {
        auto date_now = Date{std::chrono::floor<std::chrono::days>(
            std::chrono::system_clock::now())};
        auto date_in_card = card->get_due_date();
        due_date_label.remove_css_class(last_due_date_label_css_class);

        if (card->get_complete()) {
            last_due_date_label_css_class = "due-date-complete";
        } else if (date_in_card < date_now) {
            last_due_date_label_css_class = "past-due-date";
        } else {
            last_due_date_label_css_class = "due-date";
        }

        due_date_label.add_css_class(last_due_date_label_css_class);
    }
}

void CardWidget::update_complete_tasks_style(unsigned long n_complete_tasks) {
    if (!last_complete_tasks_label_css_class.empty())
        complete_tasks_label.remove_css_class(
            last_complete_tasks_label_css_class);

    if (n_complete_tasks == card->get_tasks().size()) {
        last_complete_tasks_label_css_class =
            "complete-tasks-indicator-complete";
    } else if (n_complete_tasks < card->get_tasks().size() / 2.0F ||
               card->get_tasks().size() == 1) {
        last_complete_tasks_label_css_class =
            "complete-tasks-indicator-incomplete";
    } else if (n_complete_tasks >= card->get_tasks().size() / 2.0F) {
        last_complete_tasks_label_css_class = "complete-tasks-indicator-almost";
    }
    complete_tasks_label.add_css_class(last_complete_tasks_label_css_class);
}

void CardWidget::setup_drag_and_drop() {
    // DragSource Settings
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    drag_source_c->signal_prepare().connect(
        [this, drag_source_c](double x, double y) {
            Glib::Value<CardWidget*> value_new_cardptr;
            value_new_cardptr.init(Glib::Value<CardWidget*>::value_type());
            value_new_cardptr.set(this);
            auto card_icon = Gtk::WidgetPaintable::create(*this);
            drag_source_c->set_icon(card_icon, x, y);
            return Gdk::ContentProvider::create(value_new_cardptr);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref) {
            this->cardlist_p->board.set_on_scroll();

            spdlog::get("ui")->debug("CardWidget \"{}\" has started dragging",
                                     card->get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref,
               Gdk::DragCancelReason reason) {
            this->cardlist_p->board.set_on_scroll(false);

            spdlog::get("ui")->debug("CardWidget \"{}\" has cancelled dragging",
                                     card->get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref, bool s) {
            this->cardlist_p->board.set_on_scroll(false);

            spdlog::get("ui")->debug("CardWidget \"{}\" has ended dragging",
                                     card->get_name());
        });
    add_controller(drag_source_c);

    // DropTarget Settings
    auto drop_target_c = Gtk::DropTarget::create(
        Glib::Value<CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_c->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->cardlist_p->board.set_on_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<CardWidget*>::value_type())) {
                Glib::Value<CardWidget*> dropped_value;
                dropped_value.init(value.gobj());
                auto dropped_card = dropped_value.get();

                if (dropped_card == this) {
                    spdlog::warn("Dropped CardWidget \"{}\" onto itself",
                                 card->get_name());

                    // After dropping, the receiver card still has the style set
                    // by DropControllerMotion. Reset
                    this->remove_css_class("card-to-drop");
                    return true;
                }

                if (this->cardlist_p->is_child(dropped_card)) {
                    this->cardlist_p->reorder(*dropped_card, *this);
                } else {
                    auto card_from_dropped = dropped_card->get_card();
                    CardWidget* dropped_copy =
                        this->cardlist_p->add(*card_from_dropped);
                    dropped_card->remove_from_parent();
                    this->cardlist_p->reorder(*dropped_copy, *this);
                }

                // After dropping, the receiver card still has the style set by
                // DropControllerMotion. Reset
                this->remove_css_class("card-to-drop");

                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_c);
}

void CardWidget::open_color_dialog() {
    color_dialog->choose_rgba(
        *static_cast<Gtk::Window*>(get_root()),
        [this](const Glib::RefPtr<Gio::AsyncResult>& result) {
            try {
                set_color(color_dialog->choose_rgba_finish(result));
            } catch (Gtk::DialogError& err) {
                spdlog::get("ui")->warn(
                    "CardWidget \"{}\" has failed to set color: {}",
                    card->get_name(), err.what());
            }
        });

    spdlog::get("ui")->debug("CardWidget \"{}\" has opened color dialog",
                             card->get_name());
}

void CardWidget::open_card_details_dialog() {
    auto& parent_window = *(static_cast<ProgressWindow*>(get_root()));
    parent_window.show_card_dialog(this);
}

void CardWidget::on_rename() {
    card_entry_revealer.set_reveal_child(true);
    card_label.set_visible(false);
    card_entry.grab_focus();

    spdlog::get("ui")->debug("CardWidget \"{}\" has entered rename mode",
                             card->get_name());
}

void CardWidget::off_rename() {
    card_label.set_visible();
    card_entry_revealer.set_reveal_child(false);

    spdlog::get("ui")->debug("CardWidget \"{}\" has exited rename mode",
                             card->get_name());
}

void CardWidget::clear_color() {
    card_cover_revealer.set_reveal_child(false);
    card->set_color(NO_COLOR);
}

void CardWidget::on_confirm_changes() {
    if (card_entry.get_text().compare(card_label.get_label()) != 0) {
        card->set_name(card_entry.get_text());
        card_label.set_label(card_entry.get_text());
    }
    is_new = false;
}

void CardWidget::on_cancel_changes() {
    if (is_new) {
        remove_from_parent();
    }
}

void CardWidget::set_color(const Gdk::RGBA& color) {
    card->set_color(Color{color.get_red() * 255, color.get_green() * 255,
                          color.get_blue() * 255, 1.0});
    _set_color(color);

    spdlog::get("ui")->debug("CardWidget \"{}\" has set color to {}",
                             card->get_name(), color.to_string().c_str());
}

std::string CardWidget::create_details_text() const {
    using namespace std::chrono;

    std::ostringstream details_text;

    if (!card->get_tasks().empty()) {
        details_text << Glib::ustring::compose(_("%1%% complete"),
                                               card->get_completion())
                     << "\n\n";
    }

    auto card_due_date = card->get_due_date();
    if (card_due_date.ok()) {
        auto sys_clock_now = sys_days(floor<days>(system_clock::now()));
        Date cur_date = Date{sys_clock_now};
        if (card->get_complete()) {
            details_text << _("This card is complete") << "\n\n";
        } else if (card_due_date > cur_date) {
            auto due_date_tm = system_clock::to_time_t(sys_days(card_due_date));
            char date_info[100];
            strftime(date_info, 100, _("This card is due %x"),
                     std::gmtime(&due_date_tm));
            details_text << std::string{date_info} << "\n\n";
        } else if (card_due_date == cur_date) {
            details_text << _("The card is due today") << "\n\n";
        } else {
            auto now = sys_days(floor<days>(system_clock::now()));
            auto days = sys_days(card_due_date);
            auto delta_days = -(days - now).count();

            if (delta_days < 30) {
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext(
                               "This card is past due date %1 day ago",
                               "This card is past due date %1 days ago",
                               delta_days)),
                           delta_days)
                    << "\n\n";
            } else if (delta_days > 30 && delta_days < 365) {
                long months_from_delta =
                    delta_days / 30;  // Assume every month has 30 days
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext(
                               "This card is past due date %1 month ago",
                               "This card is past due date %1 months ago",
                               months_from_delta)),
                           months_from_delta)
                    << "\n\n";
            } else if (delta_days >= 365) {
                long years_from_delta = delta_days / 365;  // Ignore leap years
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext(
                               "This card is past due date %1 year ago",
                               "This card is past due date %1 years ago",
                               years_from_delta)),
                           years_from_delta)
                    << "\n\n";
            }
        }
    }

    if (!card->get_notes().empty()) {
        details_text << _("Notes") << ":\n" << card->get_notes() << "\n\n";
    }

    std::string final_text = details_text.str();

    if (!final_text.empty()) {
        final_text.resize(final_text.size() - 2);
    }

    return final_text;
}

void CardWidget::_set_color(const Gdk::RGBA& color) {
    auto color_frame_pixbuf = Gdk::Pixbuf::create(
        Gdk::Colorspace::RGB, false, 8, CardlistWidget::CARDLIST_MAX_WIDTH, 30);
    color_frame_pixbuf->fill(rgb_to_hex(card->get_color()));

    if (card_cover_picture.get_paintable()) {
        card_cover_picture.set_paintable(nullptr);
    }

    card_cover_picture.set_paintable(
        Gdk::Texture::create_for_pixbuf(color_frame_pixbuf));
    card_cover_revealer.set_reveal_child(true);
}

void CardWidget::cleanup() { root_box.unparent(); }
}  // namespace ui
