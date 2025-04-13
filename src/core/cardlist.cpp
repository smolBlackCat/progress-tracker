#include "cardlist.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "guid.hpp"

CardList::CardList(const std::string& name)
    : Item{name, xg::newGuid()}, cards{} {}

CardList::CardList(const std::string& name, const xg::Guid uuid)
    : Item{name, uuid}, cards{} {}

bool CardList::get_modified() const { return modified || cards.get_modified(); }

ItemContainer<Card>& CardList::container() { return cards; }
