#include "card.h"

#include <iostream>

#include "cardlist-widget.h"

ui::CardWidget::CardWidget(std::shared_ptr<Card> card_refptr)
    : ui::EditableLabelHeader{card_refptr->get_name()},
      card_refptr{card_refptr},
      cardlist_p{nullptr} {
    add_option("remove", "Remove",
               sigc::mem_fun(*this, &ui::CardWidget::remove));
}

void ui::CardWidget::remove() {
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

void ui::CardWidget::on_confirm_changes() {
    card_refptr->set_name(label.get_text());
}