#include "cardlist-widget.h"

#include <format>
#include <iostream>

#include "board-widget.h"
#include "card.h"

ui::CardListHeader::CardListHeader(std::shared_ptr<CardList>& cardlist_refptr)
    : EditableLabelHeader{}, cardlist_refptr{cardlist_refptr} {
    set_label(cardlist_refptr->get_name());
    label.set_name("cardlist-title");
    entry.set_name("cardlist-title");
}

void ui::CardListHeader::on_confirm_changes() {
    cardlist_refptr->set_name(label.get_text());
}

ui::CardlistWidget::CardlistWidget(BoardWidget& board,
                       std::shared_ptr<CardList> cardlist_refptr)
    : Gtk::ListBox{},
      add_card_button{"Add card"},
      root{Gtk::Orientation::VERTICAL},
      cards_tracker{},
      board{board},
      cardlist_refptr{cardlist_refptr},
      cardlist_header{cardlist_refptr} {
    add_css_class("rich-list");
    set_valign(Gtk::Align::START);
    set_vexpand(true);
    set_halign(Gtk::Align::START);
    set_size_request(CARDLIST_SIZE, CARDLIST_SIZE * 2);
    set_selection_mode(Gtk::SelectionMode::NONE);

    cardlist_header.add_option(
        "remove", "Remove", [this]() { this->board.remove_cardlist(*this); });

    add_card_button.set_valign(Gtk::Align::CENTER);
    add_card_button.set_halign(Gtk::Align::START);
    add_card_button.set_hexpand(false);
    add_card_button.signal_clicked().connect([this]() { add_card(); });
    root.append(add_card_button);

    append(cardlist_header);
    for (auto& card : cardlist_refptr->get_card_vector()) {
        add_card(card);
    }

    root.set_size_request(CARDLIST_SIZE, CARDLIST_SIZE);
    root.set_valign(Gtk::Align::FILL);
    root.set_vexpand();
    root.set_spacing(4);
    root.set_margin_top(4);

    Gtk::ScrolledWindow scr_window{};
    scr_window.set_child(root);
    scr_window.set_size_request(CARDLIST_SIZE, CARDLIST_SIZE * 2);
    scr_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    append(scr_window);

    // Makes the header and the list itself non-selectable
    get_row_at_index(0)->set_activatable(false);
    get_row_at_index(1)->set_activatable(false);
}

void ui::CardlistWidget::reorder_card(ui::CardWidget& next, ui::CardWidget& sibling) {
    root.reorder_child_after(next, sibling);
    cardlist_refptr->reorder_card(next.get_card(), sibling.get_card());
}

void ui::CardlistWidget::setup_drag_and_drop(ui::CardWidget* card) {
    // DragSource Settings
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    drag_source_c->signal_prepare().connect(
        [card, drag_source_c](double x, double y) {
            Glib::Value<ui::CardWidget*> value_new_cardptr;
            value_new_cardptr.init(Glib::Value<ui::CardWidget*>::value_type());
            value_new_cardptr.set(card);
            auto card_paintable_widget = Gtk::WidgetPaintable::create(*card);
            drag_source_c->set_icon(card_paintable_widget, x, y);
            return Gdk::ContentProvider::create(value_new_cardptr);
        }, false
    );
    card->add_controller(drag_source_c);

    // DropTarget Settings
    auto drop_target_c = Gtk::DropTarget::create(
        Glib::Value<ui::CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_c->signal_drop().connect(
        [this, card](const Glib::ValueBase& value, double x, double y) {
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardWidget*>::value_type())) {
                Glib::Value<ui::CardWidget*> dropped_value;
                dropped_value.init(value.gobj());
                auto dropped_card = dropped_value.get();
                if (is_child(dropped_card)) {
                    reorder_card(*dropped_card, *card);
                } else {
                    // Dropped card's parent is the dropped-on card's parent
                    auto card_refptr = dropped_card->get_card();
                    cardlist_refptr->add_card(*card_refptr);
                    dropped_card->remove();
                    auto card_from_dropped = add_card(card_refptr);
                    reorder_card(*card_from_dropped, *card);
                }
                return true;
            }
            return false;
        },
        false);
    card->add_controller(drop_target_c);
}

void ui::CardlistWidget::remove_card(ui::CardWidget* card) {
    root.remove(*card);
    cardlist_refptr->remove_card(*card->get_card());

    for (size_t i = 0; i < cards_tracker.size(); i++) {
        if (cards_tracker[i] == card) {
            cards_tracker.erase(cards_tracker.begin() + i);
        }
    }
}

ui::CardWidget* ui::CardlistWidget::add_card(std::shared_ptr<Card> card) {
    auto new_card = Gtk::make_managed<ui::CardWidget>(card);
    cards_tracker.push_back(new_card);
    new_card->set_cardlist(this);
    root.append(*new_card);
    root.reorder_child_after(add_card_button, *new_card);
    setup_drag_and_drop(new_card);
    return new_card;
}

ui::CardWidget* ui::CardlistWidget::add_card() {
    return add_card(cardlist_refptr->add_card(Card{"New Card"}));
}

std::shared_ptr<CardList>& ui::CardlistWidget::get_cardlist_refptr() {
    return cardlist_refptr;
}

bool ui::CardlistWidget::is_child(ui::CardWidget* card) {
    for (auto& card_ : cards_tracker) {
        if (card == card_) return true;
    }
    return false;
}

ui::CardListHeader& ui::CardlistWidget::get_header() { return cardlist_header; }