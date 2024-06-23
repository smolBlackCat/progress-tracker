#include "card.h"

#include <glibmm/i18n.h>

#include "cardlist-widget.h"

ui::CardWidget::CardWidget(std::shared_ptr<Card> card_refptr, bool is_new)
    : ui::EditableLabelHeader{card_refptr->get_name()},
      card_refptr{card_refptr},
      cardlist_p{nullptr},
      is_new{is_new} {
    add_css_class("card");
    add_option("remove", _("Remove"),
               sigc::mem_fun(*this, &ui::CardWidget::remove_from_parent));

    signal_confirm().connect([this](std::string label) {
        this->card_refptr->set_name(label);
        this->is_new = false;
    });
    signal_cancel().connect([this](std::string label) {
        if (this->is_new) {
            this->remove_from_parent();
        }
    });
    setup_drag_and_drop();
}

void ui::CardWidget::remove_from_parent() {
    if (cardlist_p) {
        cardlist_p->remove_card(this);
    }
}

void ui::CardWidget::set_cardlist(ui::CardlistWidget* cardlist_p) {
    if (cardlist_p) {
        this->cardlist_p = cardlist_p;
    }
}

std::shared_ptr<Card> ui::CardWidget::get_card() { return card_refptr; }

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
            this->cardlist_p->board.on_drag = true;
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref,
               Gdk::DragCancelReason reason) {
            this->cardlist_p->board.on_drag = false;
            return true;
        },
        false);
    add_controller(drag_source_c);

    // DropTarget Settings
    auto drop_target_c = Gtk::DropTarget::create(
        Glib::Value<ui::CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_c->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->cardlist_p->board.on_drag = false;
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
