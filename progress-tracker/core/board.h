#pragma once

#include <vector>
#include "item.h"
#include "cardlist.h"


class Board : public Item {

public:
    Board(std::string name, std::string background);

    bool set_background(std::string other);
    std::string get_background() const;
    bool add_cardlist(CardList& cardlist);
    bool remove_cardlist(CardList& cardlist);

private:
    std::string background;
    std::vector<CardList> cardlist_vector;

    bool is_background(std::string& background) const;
};