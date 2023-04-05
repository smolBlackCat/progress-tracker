#pragma once

#include <vector>
#include "item.h"
#include "cardlist.h"


class Board : public Item {

public:
    Board(std::string name, std::string background);
    Board(std::string name, std::string background, unsigned long long id);

    bool set_background(std::string other);
    std::string get_background() const;
    std::string fetch_xml() const override;
    bool add_cardlist(CardList& cardlist);
    bool remove_cardlist(CardList& cardlist);

private:
    std::string background;
    std::vector<CardList> cardlist_vector;

    bool is_background(std::string& background) const;
};