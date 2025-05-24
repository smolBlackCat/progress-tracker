#include "cardlist.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "guid.hpp"

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
