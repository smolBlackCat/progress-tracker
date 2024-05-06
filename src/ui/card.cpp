#include "card.h"

#include <glibmm/i18n.h>

#include <iostream>

#include "cardlist-widget.h"

ui::CardWidget::CardWidget(std::shared_ptr<Card> card_refptr, bool is_new)
    : ui::EditableLabelHeader{card_refptr->get_name()},
      card_refptr{card_refptr},
      cardlist_p{nullptr},
      is_new{is_new} {
    set_name("card");
    add_option("remove", _("Remove"),
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
    EditableLabelHeader::on_confirm_changes();
    card_refptr->set_name(label.get_text());
    is_new = false;
}

void ui::CardWidget::on_cancel_changes() {
    EditableLabelHeader::on_cancel_changes();
    if (is_new) {
        remove();
    }
}