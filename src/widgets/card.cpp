#include "card.h"

#include <dialog/card-dialog.h>
#include <glibmm/i18n.h>
#include <utils.h>

#include <chrono>
#include <numeric>

#include "cardlist-widget.h"

ui::CardWidget::CardWidget(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& builder,
                           std::shared_ptr<Card> card, bool is_new)
    : Gtk::Box{cobject},
      card{card},
      cardlist_p{nullptr},
      is_new{is_new},
      card_cover_revealer{
          builder->get_widget<Gtk::Revealer>("card-cover-revealer")},
      card_entry_revealer{
          builder->get_widget<Gtk::Revealer>("card-entry-revealer")},
      card_cover_picture{
          builder->get_widget<Gtk::Picture>("card-cover-picture")},
      card_label{builder->get_widget<Gtk::Label>("card-label")},
      complete_tasks_label{
          builder->get_widget<Gtk::Label>("complete-tasks-label")},
      due_date_label{builder->get_widget<Gtk::Label>("due-date-label")},
      card_entry{builder->get_widget<Gtk::Entry>("card-entry")},
      card_menu_button{
          builder->get_widget<Gtk::MenuButton>("card-menu-button")},
      popover_menu{},
      key_controller{Gtk::EventControllerKey::create()},
      card_label_click_controller{Gtk::GestureClick::create()},
      focus_controller{Gtk::EventControllerFocus::create()},
      click_controller{Gtk::GestureClick::create()},
      card_menu_model{builder->get_object<Gio::MenuModel>("card-menu-model")},
      color_dialog{Gtk::ColorDialog::create()} {
    set_title(card->get_name());
    if (is_new) {
        on_rename();  // Open card on rename mode by default whenever a new card
                      // is created
    }

    auto card_actions = Gio::SimpleActionGroup::create();
    card_actions->add_action("rename",
                             sigc::mem_fun(*this, &ui::CardWidget::on_rename));
    card_actions->add_action(
        "details",
        sigc::mem_fun(*this, &ui::CardWidget::open_card_details_dialog));
    card_actions->add_action(
        "set-color", sigc::mem_fun(*this, &ui::CardWidget::open_color_dialog));
    card_actions->add_action("unset-color",
                             sigc::mem_fun(*this, &CardWidget::clear_color));
    card_actions->add_action(
        "remove", sigc::mem_fun(*this, &ui::CardWidget::remove_from_parent));
    card_menu_button->insert_action_group("card", card_actions);

    card_cover_revealer->property_child_revealed().signal_changed().connect(
        [this]() {
            if (!card_cover_revealer->get_child_revealed())
                this->card_cover_picture->set_paintable(nullptr);
        });

    popover_menu.set_parent(*this);
    popover_menu.set_menu_model(card_menu_model);
    popover_menu.insert_action_group("card", card_actions);

    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (card_entry_revealer->get_child_revealed()) {
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
            if (n_pressed >= 1 && !card_entry_revealer->get_child_revealed() &&
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

    focus_controller->signal_leave().connect([this]() {
        this->on_confirm_changes();
        this->off_rename();
    });

    card_entry->add_controller(key_controller);
    card_entry->add_controller(focus_controller);
    card_label->add_controller(card_label_click_controller);
    add_controller(click_controller);

    if (card->is_color_set()) {
        Color card_color = card->get_color();
        set_color(Gdk::RGBA{static_cast<float>(std::get<0>(card_color)) / 255,
                            static_cast<float>(std::get<1>(card_color)) / 255,
                            static_cast<float>(std::get<2>(card_color)) / 255,
                            std::get<3>(card_color)});

        if (!is_new) {
            card->set_modified(false);
        }
    }

    update_due_date();
    update_complete_tasks();

    set_tooltip_text(create_details_text());
    setup_drag_and_drop();
}

void ui::CardWidget::set_title(const std::string& label) {
    card_label->set_label(label);
    card_entry->set_text(label);
}

std::string ui::CardWidget::get_title() const { return card_label->get_label(); }

void ui::CardWidget::remove_from_parent() {
    if (cardlist_p) {
        cardlist_p->remove(this);
    }
}

void ui::CardWidget::set_cardlist(ui::CardlistWidget* cardlist_p) {
    if (cardlist_p) {
        this->cardlist_p = cardlist_p;
    }
}

std::shared_ptr<Card> ui::CardWidget::get_card() { return card; }

ui::CardlistWidget const* ui::CardWidget::get_cardlist_widget() const {
    return cardlist_p;
}

void ui::CardWidget::update_complete_tasks() {
    if (card->get_tasks().empty()) {
        complete_tasks_label->set_label("");
        complete_tasks_label->set_visible(false);
    } else {
        complete_tasks_label->set_visible();
        float n_complete_tasks =
            std::accumulate(card->get_tasks().begin(), card->get_tasks().end(),
                            0, [](int acc, const std::shared_ptr<Task>& task) {
                                return task->get_done() ? ++acc : acc;
                            });
        complete_tasks_label->set_label(
            std::format("{}/{}", n_complete_tasks, card->get_tasks().size()));

        // Silences warning about deleting empty css classes
        update_complete_tasks_style(n_complete_tasks);
    }
}

void ui::CardWidget::update_due_date() {
    if (card->get_due_date().ok()) {
        due_date_label->set_visible();
        auto sys_days = std::chrono::sys_days(card->get_due_date());
        sys_days++;
        std::time_t time = std::chrono::system_clock::to_time_t(sys_days);

        char date_str[255];
        strftime(date_str, 255, "%d %b, %Y", std::localtime(&time));
        due_date_label->set_label(_("Due: ") + Glib::ustring{date_str});

        update_due_date_label_style();
    } else {
        due_date_label->set_visible(false);
    }
}

void ui::CardWidget::update_due_date_label_style() {
    auto date_now = Date{std::chrono::floor<std::chrono::days>(
        std::chrono::system_clock::now())};
    auto date_in_card = card->get_due_date();
    due_date_label->remove_css_class(last_due_date_label_css_class);

    if (card->get_complete()) {
        last_due_date_label_css_class = "due-date-complete";
    } else if (date_in_card < date_now) {
        last_due_date_label_css_class = "past-due-date";
    } else {
        last_due_date_label_css_class = "due-date";
    }

    due_date_label->add_css_class(last_due_date_label_css_class);
}

void ui::CardWidget::update_complete_tasks_style(
    unsigned long n_complete_tasks) {
    if (!last_complete_tasks_label_css_class.empty())
        complete_tasks_label->remove_css_class(
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
    complete_tasks_label->add_css_class(last_complete_tasks_label_css_class);
}

void ui::CardWidget::setup_drag_and_drop() {
    // DragSource Settings
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    drag_source_c->signal_prepare().connect(
        [this, drag_source_c](double x, double y) {
            Glib::Value<ui::CardWidget*> value_new_cardptr;
            value_new_cardptr.init(Glib::Value<ui::CardWidget*>::value_type());
            value_new_cardptr.set(this);
            auto card_icon = Gtk::WidgetPaintable::create(*this);
            drag_source_c->set_icon(card_icon, x, y);
            return Gdk::ContentProvider::create(value_new_cardptr);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref) {
            this->cardlist_p->board.set_on_scroll();
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref,
               Gdk::DragCancelReason reason) {
            this->cardlist_p->board.set_on_scroll(false);
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref, bool s) {
            this->cardlist_p->board.set_on_scroll(false);
        });
    add_controller(drag_source_c);

    // DropTarget Settings
    auto drop_target_c = Gtk::DropTarget::create(
        Glib::Value<ui::CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_c->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->cardlist_p->board.set_on_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardWidget*>::value_type())) {
                Glib::Value<ui::CardWidget*> dropped_value;
                dropped_value.init(value.gobj());
                auto dropped_card = dropped_value.get();

                if (dropped_card == this) {
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
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_c);
}

void ui::CardWidget::open_color_dialog() {
    color_dialog->choose_rgba(
        *static_cast<Gtk::Window*>(get_root()),
        [this](const Glib::RefPtr<Gio::AsyncResult>& result) {
            try {
                set_color(color_dialog->choose_rgba_finish(result));
            } catch (Gtk::DialogError& err) {
                // FIXME: This would be a good opportunity to have a logging
                // system here
            }
        });
}

void ui::CardWidget::open_card_details_dialog() {
    auto& parent_window = *(static_cast<Gtk::Window*>(get_root()));
    auto card_details_dialog = CardDetailsDialog::create(*this);
    card_details_dialog->open(parent_window);
}

void ui::CardWidget::on_rename() {
    card_entry_revealer->set_reveal_child(true);
    card_label->set_visible(false);
}

void ui::CardWidget::off_rename() {
    card_label->set_visible();
    card_entry_revealer->set_reveal_child(false);
}

void ui::CardWidget::clear_color() {
    card_cover_revealer->set_reveal_child(false);
    card->set_color(NO_COLOR);
}

void ui::CardWidget::on_confirm_changes() {
    if (card_entry->get_text().compare(card_label->get_label()) != 0) {
        card->set_name(card_entry->get_text());
        card_label->set_label(card_entry->get_text());
    }
    is_new = false;
}

void ui::CardWidget::on_cancel_changes() {
    if (is_new) {
        remove_from_parent();
    }
}

void ui::CardWidget::set_color(const Gdk::RGBA& color) {
    auto color_frame_pixbuf = Gdk::Pixbuf::create(
        Gdk::Colorspace::RGB, false, 8, CardlistWidget::CARDLIST_MAX_WIDTH, 30);
    card->set_color(Color{color.get_red() * 255, color.get_green() * 255,
                          color.get_blue() * 255, 1.0});
    color_frame_pixbuf->fill(rgb_to_hex(card->get_color()));
    if (card_cover_picture->get_paintable()) {
        card_cover_picture->set_paintable(nullptr);
    }
    card_cover_picture->set_paintable(
        Gdk::Texture::create_for_pixbuf(color_frame_pixbuf));
    card_cover_revealer->set_reveal_child(true);
}

std::string ui::CardWidget::create_details_text() const {
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
            auto due_date_tm =
                system_clock::to_time_t(++sys_days(card_due_date));
            char date_info[100];
            strftime(date_info, 100, _("This card is due %x"),
                     std::localtime(&due_date_tm));
            details_text << std::string{date_info} << "\n\n";
        } else if (card_due_date == cur_date) {
            details_text << _("The card is due today") << "\n\n";
        } else {
            auto now = sys_days(floor<days>(system_clock::now()));
            auto days = sys_days(card_due_date);
            auto delta_days = -(days - now).count();

            if (delta_days < 30) {
                details_text << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext("This card is past due date %1 day ago",
                                    "This card is past due date %1 days ago",
                                    delta_days)),
                           delta_days) << "\n\n";
            } else if (delta_days > 30 && delta_days < 365) {
                long months_from_delta =
                    delta_days / 30;  // Assume every month has 30 days
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext("This card is past due date %1 month ago",
                                    "This card is past due date %1 months ago",
                                    months_from_delta)),
                           months_from_delta)
                    << "\n\n";
            } else if (delta_days >= 365) {
                long years_from_delta = delta_days / 365;  // Ignore leap years
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext("This card is past due date %1 year ago",
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
