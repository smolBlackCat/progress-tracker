#include "cardlist.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "guid.hpp"

CardList::CardList(const std::string& name)
    : Item{name, xg::newGuid()}, ItemContainer{}, cards{} {}

CardList::CardList(const std::string& name, const xg::Guid uuid)
    : Item{name, uuid}, ItemContainer{}, cards{} {}

void CardList::set_modified(bool modified) {
    Item::set_modified(modified);
    ItemContainer::set_modified(modified);
}

bool CardList::get_modified() const {
    return Item::get_modified() || ItemContainer::get_modified();
}
