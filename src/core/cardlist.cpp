#include "cardlist.h"

#include "guid.hpp"

std::shared_ptr<CardList> CardList::create(const std::string& name) {
    return std::shared_ptr<CardList>(new CardList{name});
}

std::shared_ptr<CardList> CardList::create(const std::string& name,
                                           const xg::Guid& uuid) {
    return std::shared_ptr<CardList>(new CardList{name, uuid});
}

CardList::CardList(const std::string& name) : CardList{name, xg::newGuid()} {}

CardList::CardList(const std::string& name, const xg::Guid uuid)
    : Item{name, uuid}, cards{} {}

void CardList::set_name(const std::string& name) {
    Item::set_name(name);
    modify();
}

CardList::~CardList() {}

void CardList::modify(bool m) { m_modified = m; }

bool CardList::modified() const { return m_modified || cards.modified(); }

ItemContainer<Card>& CardList::container() { return cards; }
