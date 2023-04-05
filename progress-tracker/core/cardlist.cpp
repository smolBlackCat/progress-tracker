#include "cardlist.h"

CardList::CardList(std::string name) : Item{name}, card_vector{} {
    
}

CardList::CardList(std::string name, unsigned long long id) : Item{name, id}, card_vector{} {

}

bool CardList::add_card(Card& card) {
    try {
        card_vector.push_back(card);
        return true;
    } catch (std::bad_alloc e) {
        return false;
    }
}

bool CardList::remove_card(Card& card) {
    for (size_t i = 0; i < card_vector.size(); i++) {
        if (card == card_vector.at(i)) {
            card_vector.erase(card_vector.begin()+i);
            return true;
        }
    }
    return false;
}

std::string CardList::fetch_xml() const {
    std::string cardlist_xml = "\t<list name=\"" + name + "\">\n";

    for (size_t i = 0; i < card_vector.size(); i++) {
        cardlist_xml += card_vector.at(i).fetch_xml() + "\n";
    }
    cardlist_xml += "\t</list>";

    return cardlist_xml;
}