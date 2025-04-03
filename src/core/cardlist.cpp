#include "cardlist.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "guid.hpp"

CardList::CardList(const std::string& name)
    : Item{name, xg::newGuid()}, cards{} {}

CardList::CardList(const std::string& name, const xg::Guid uuid)
    : Item{name, uuid}, cards{} {}

std::shared_ptr<Card> CardList::add(const Card& card) {
    std::shared_ptr<Card> new_card{new Card{card}};
    if (new_card) {
        spdlog::get("core")->info(
            "[CardList] Cardlist \"{}\" has added Card \"{}\"", name,
            new_card->get_name());
        cards.push_back(new_card);
        modified = true;
    }
    return new_card;
}

bool CardList::remove(const Card& card) {
    for (size_t i = 0; i < cards.size(); i++) {
        if (card == *cards.at(i)) {
            cards.erase(cards.begin() + i);
            modified = true;
            spdlog::get("core")->info(
                "[CardList] Cardlist \"{}\" removed Card \"{}\"", name,
                card.get_name());
            return true;
        }
    }
    spdlog::get("core")->warn(
        "[CardList] Cardlist \"{}\" cannot remove Card \"{}\" because it is "
        "not there",
        name, card.get_name());
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

ReorderingType CardList::reorder(const Card& next, const Card& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    ssize_t c = 0;
    for (auto& card : cards) {
        if (*card == next) {
            next_i = c;
        } else if (*card == sibling) {
            sibling_i = c;
        }
        c++;
    }

    bool any_absent = next_i == -1 || sibling_i == -1;
    bool is_same = next_i == sibling_i;

    if (any_absent || is_same) {
        spdlog::get("core")->warn(
            "[CardList] Cannot reorder cards: same references or missing");
        return ReorderingType::INVALID;
    }

    std::shared_ptr<Card> next_v = cards[next_i];
    cards.erase(cards.begin() + next_i);

    ReorderingType reordering;
    if (next_i > sibling_i) {
        // Down to up reordering
        cards.insert(cards.begin() + (sibling_i == 0 ? 0 : sibling_i), next_v);
        reordering = ReorderingType::DOWNUP;
        spdlog::get("core")->info(
            "[CardList] Reordered Card \"{}\" before Card \"{}\"",
            next.get_name(), sibling.get_name());
    } else if (next_i < sibling_i) {
        // Up to down reordering
        cards.insert(cards.begin() + sibling_i, next_v);
        reordering = ReorderingType::UPDOWN;
        spdlog::get("core")->info(
            "[CardList] Reordered Card \"{}\" after Card \"{}\"",
            next.get_name(), sibling.get_name());
    }

    modified = true;

    return reordering;
}
