#include "card.h"

#include <dialog/card-dialog.h>
#include <glibmm/i18n.h>
#include <utils.h>

#include <numeric>

#include "cardlist-widget.h"

ui::CardWidget::CardWidget(BaseObjectType* cobject,
                           const Glib::RefPtr<Gtk::Builder>& builder,
                           std::shared_ptr<Card> card_refptr, bool is_new)
    : Gtk::Box{cobject},
      card_refptr{card_refptr},
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
      key_controller{Gtk::EventControllerKey::create()},
      click_controller{Gtk::GestureClick::create()},
      card_menu_model{builder->get_object<Gio::MenuModel>("card-menu-model")},
      color_dialog{Gtk::ColorDialog::create()} {
    set_label(card_refptr->get_name());
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

    click_controller->signal_released().connect(
        [this](int n_pressed, double x, double y) {
            if (n_pressed >= 1 && !card_entry_revealer->get_child_revealed()) {
                this->on_rename();
            }
        });

    card_entry->add_controller(key_controller);
    card_label->add_controller(click_controller);

    if (card_refptr->is_color_set()) {
        Color card_color = card_refptr->get_color();
        set_color(Gdk::RGBA{static_cast<float>(std::get<0>(card_color)) / 255,
                            static_cast<float>(std::get<1>(card_color)) / 255,
                            static_cast<float>(std::get<2>(card_color)) / 255,
                            std::get<3>(card_color)});

        if (!is_new) {
            card_refptr->set_modified(false);
        }
    }

    set_tooltip_text(create_details_text());
    setup_drag_and_drop();

    if (!card_refptr->get_tasks().empty()) {
        complete_tasks_label->set_visible();
        update_completed();
    }
}

void ui::CardWidget::set_label(const std::string& label) {
    card_label->set_label(label);
    card_entry->set_text(label);
}

std::string ui::CardWidget::get_text() const { return card_label->get_label(); }

void ui::CardWidget::remove_from_parent() {
    if (cardlist_p) {
        cardlist_p->remove_card(this);
    }

    // The pixbuf in color_frame is invalid on drag and drops events.
    // Thus it is important to clean whatever existent instances.
    // color_frame.set_pixbuf(nullptr);
}

void ui::CardWidget::set_cardlist(ui::CardlistWidget* cardlist_p) {
    if (cardlist_p) {
        this->cardlist_p = cardlist_p;
    }
}

std::shared_ptr<Card> ui::CardWidget::get_card() { return card_refptr; }

ui::CardlistWidget const* ui::CardWidget::get_cardlist_widget() const {
    return cardlist_p;
}

void ui::CardWidget::update_completed() {
    if (card_refptr->get_tasks().empty()) {
        complete_tasks_label->set_label("");
        complete_tasks_label->set_visible(false);
    } else {
        complete_tasks_label->set_visible();
        float n_complete_tasks = std::accumulate(
            card_refptr->get_tasks().begin(), card_refptr->get_tasks().end(), 0,
            [](int acc, const std::shared_ptr<Task>& task) {
                return task->get_done() ? ++acc : acc;
            });
        complete_tasks_label->set_label(std::format(
            "{}/{}", n_complete_tasks, card_refptr->get_tasks().size()));

        // Silences warning about deleting empty css classes
        if (!last_complete_tasks_label_css_class.empty())
            complete_tasks_label->remove_css_class(
                last_complete_tasks_label_css_class);

        if (n_complete_tasks < card_refptr->get_tasks().size() / 2 ||
            card_refptr->get_tasks().size() == 1) {
            last_complete_tasks_label_css_class =
                "complete-tasks-indicator-incomplete";
        } else if (n_complete_tasks == card_refptr->get_tasks().size()) {
            last_complete_tasks_label_css_class =
                "complete-tasks-indicator-complete";
        } else if (n_complete_tasks >= card_refptr->get_tasks().size() / 2) {
            last_complete_tasks_label_css_class =
                "complete-tasks-indicator-almost";
        }
        complete_tasks_label->add_css_class(
            last_complete_tasks_label_css_class);
    }
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
                    this->cardlist_p->reorder_cardwidget(*dropped_card, *this);
                } else {
                    auto card_from_dropped = dropped_card->get_card();
                    CardWidget* dropped_copy =
                        this->cardlist_p->add_card(*card_from_dropped);
                    dropped_card->remove_from_parent();
                    this->cardlist_p->reorder_cardwidget(*dropped_copy, *this);
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
    auto card_details_dialog = CardDetailsDialog::create(*this);
    card_details_dialog->set_transient_for(
        *(static_cast<Gtk::Window*>(get_root())));
    card_details_dialog->set_hide_on_close(false);
    card_details_dialog->set_modal();
    card_details_dialog->set_visible();
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
    card_refptr->set_color(NO_COLOR);
}

void ui::CardWidget::on_confirm_changes() {
    // FIXME: Use Glib::ustring::compare here
    if (card_entry->get_text() != card_label->get_label()) {
        card_refptr->set_name(card_entry->get_text());
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
    card_refptr->set_color(Color{color.get_red() * 255, color.get_green() * 255,
                                 color.get_blue() * 255, 1.0});
    color_frame_pixbuf->fill(rgb_to_hex(card_refptr->get_color()));
    card_cover_picture->set_paintable(
        Gdk::Texture::create_for_pixbuf(color_frame_pixbuf));
    card_cover_revealer->set_reveal_child(true);
}

std::string ui::CardWidget::create_details_text() const {
    std::ostringstream details_text;

    if (!card_refptr->get_tasks().empty()) {
        details_text << Glib::ustring::compose(_("%1%% complete"),
                                               card_refptr->get_completion())
                     << "\n\n";
    }

    if (!card_refptr->get_notes().empty()) {
        details_text << _("Notes") << ":\n"
                     << card_refptr->get_notes() << "\n\n";
    }

    std::string final_text = details_text.str();

    if (!final_text.empty()) {
        final_text.resize(final_text.size() - 2);
    }

    return final_text;
}
