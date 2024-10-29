#include "cardlist.h"

#include <iterator>
#include <stdexcept>

CardList::CardList(const std::string& name) : Item{name}, cards{} {}

std::shared_ptr<Card> CardList::add_card(const Card& card) {
    std::shared_ptr<Card> new_card{new Card{card}};
    if (new_card) {
        cards.push_back(new_card);
        modified = true;
    }
    return new_card;
}

bool CardList::remove_card(const Card& card) {
    for (size_t i = 0; i < cards.size(); i++) {
        if (card == *cards.at(i)) {
            cards.erase(cards.begin() + i);
            modified = true;
            return true;
        }
    }
    return false;
}

void CardList::set_modified(bool modified) {
    Item::set_modified(modified);

    for (auto& card : cards) {
        card->set_modified(modified);
    }
}

bool CardList::cards_modified() {
    for (auto& card : cards) {
        if (card->get_modified()) {
            return true;
        }
    }
    return false;
}

bool CardList::get_modified() { return modified || cards_modified(); }

const std::vector<std::shared_ptr<Card>>& CardList::get_cards() {
    return cards;
}

void CardList::reorder_card(const Card& next, const Card& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    for (ssize_t i = 0; i < cards.size(); i++) {
        if (*cards[i] == next) {
            next_i = i;
        }
        if (*cards[i] == sibling) {
            sibling_i = i;
        }
    }

    bool any_absent_item = next_i + sibling_i < 0;
    bool is_same_item = next_i == sibling_i;
    bool already_in_order = next_i - sibling_i == 1;
    if (any_absent_item || is_same_item || already_in_order) {
        return;
    }

    auto next_it = std::next(cards.begin(), next_i);
    std::shared_ptr<Card> temp_v = cards[next_i];
    cards.erase(next_it);

    if (next_i < sibling_i) {
        sibling_i -= 1;
    }

    if (sibling_i == cards.size() - 1) {
        cards.push_back(temp_v);
    } else {
        auto sibling_it = std::next(cards.begin(), sibling_i + 1);
        cards.insert(sibling_it, temp_v);
    }
    modified = true;
}
