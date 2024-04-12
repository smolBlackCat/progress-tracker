#include "cardlist.h"

#include <iostream>
#include <iterator>
#include <stdexcept>

CardList::CardList(const std::string& name) : Item{name}, card_vector{} {}

std::shared_ptr<Card> CardList::add_card(const Card& card) {
    std::shared_ptr<Card> new_card{new Card{card}};
    if (new_card) {
        card_vector.push_back(new_card);
        modified = true;
    }
    return new_card;
}

bool CardList::remove_card(Card& card) {
    for (size_t i = 0; i < card_vector.size(); i++) {
        if (card == *card_vector.at(i)) {
            card_vector.erase(card_vector.begin() + i);
            modified = true;
            return true;
        }
    }
    return false;
}

bool CardList::cards_modified() {
    for (auto& card : card_vector) {
        if (card->is_modified()) {
            return true;
        }
    }
    return false;
}

bool CardList::is_modified() { return modified || cards_modified(); }

std::vector<std::shared_ptr<Card>>& CardList::get_card_vector() {
    return card_vector;
}

void CardList::reorder_card(std::shared_ptr<Card> next,
                            std::shared_ptr<Card> sibling) {
    size_t next_i = -1;
    size_t sibling_i = -1;

    for (size_t i = 0; i < card_vector.size(); i++) {
        if (*card_vector[i] == *next) {
            next_i = i;
        }
        if (*card_vector[i] == *sibling) {
            sibling_i = i;
        }
    }

    if (next_i == -1 || sibling_i == -1) {
        throw std::invalid_argument{
            "Either next or sibling are not children of this cardlist"};
    }

    auto next_it = std::next(card_vector.begin(), next_i);
    std::shared_ptr<Card> temp_v = card_vector[next_i];
    card_vector.erase(next_it);

    if (next_i < sibling_i) {
        sibling_i -= 1;
    }

    if (sibling_i == card_vector.size() - 1) {
        card_vector.push_back(temp_v);
    } else {
        auto sibling_it = std::next(card_vector.begin(), sibling_i + 1);
        card_vector.insert(sibling_it, temp_v);
    }
    modified = true;
}