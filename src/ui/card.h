#pragma once

#include <gtkmm.h>

#include <memory>

#include "../core/card.h"
#include "editable-label-header.h"
#include "cardlist.h"

namespace ui {

class Cardlist;

class CardWidget : public EditableLabelHeader {
public:
    CardWidget(std::shared_ptr<Card> card_refptr);

    void remove();

    void set_cardlist(ui::Cardlist* cardlist_p);

    ui::Cardlist const* get_cardlist();

    std::shared_ptr<Card> get_card();

    bool equal(ui::CardWidget& other);

protected:
    void on_confirm_changes();

private:
    std::shared_ptr<Card> card_refptr;
    ui::Cardlist* cardlist_p;
};
}  // namespace ui