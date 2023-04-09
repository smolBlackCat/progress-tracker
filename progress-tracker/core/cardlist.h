#pragma once

#include "item.h"
#include "card.h"
#include <vector>

class CardList : public Item {

public:
    CardList(std::string name);

    bool add_card(Card& card);
    bool remove_card(Card& card);

private:
    std::vector<Card> card_vector;
};