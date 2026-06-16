#include "core/cardlist.h"

#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CardList Instantiation", "[CardList]") {
    auto cardlist = CardList::create("TODO");

    REQUIRE_FALSE(cardlist->modified());

    CHECK(cardlist->get_name() == "TODO");
    CHECK(cardlist->container().get_data().size() == 0);
}

TEST_CASE("Signal Emission", "[CardList]") {
    auto cardlist = CardList::create("TODO");
    bool name_changed = false;

    cardlist->signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });

    SECTION("Name Changing") {
        cardlist->set_name("DOING");

        CHECK(name_changed);
    }
}

TEST_CASE("Modification State", "[CardList]") {
    auto cardlist = CardList::create("TODO");

    bool name_changed, appended_item, removed_item, reordered_item;
    name_changed = appended_item = removed_item = reordered_item = false;

    cardlist->signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });

    SECTION("Name Changing") {
        cardlist->set_name("DOING");

        CHECK(cardlist->modified());
    }
}
