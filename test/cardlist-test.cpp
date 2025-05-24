#include "core/cardlist.h"

#include "core/item.h"
#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CardList Instantiation", "[CardList]") {
    CardList cardlist{"TODO"};

    REQUIRE_FALSE(cardlist.modified());

    CHECK(cardlist.get_name() == "TODO");
    CHECK(cardlist.container().get_data().size() == 0);
}

TEST_CASE("Signal Emission", "[CardList]") {
    CardList cardlist{"TODO"};

    bool name_changed, appended_item, removed_item, reordered_item;
    name_changed = appended_item = removed_item = reordered_item = false;

    cardlist.signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });
    cardlist.container().signal_append().connect(
        [&appended_item](std::shared_ptr<Card>) { appended_item = true; });
    cardlist.container().signal_remove().connect(
        [&removed_item](std::shared_ptr<Card>) { removed_item = true; });
    cardlist.container().signal_reorder().connect(
        [&reordered_item](std::shared_ptr<Card>, std::shared_ptr<Card>,
                          ReorderingType) { reordered_item = true; });

    SECTION("Name Changing") {
        cardlist.set_name("DOING");

        CHECK(name_changed);
    }

    SECTION("Name Changing from copy") {
        CardList another_one{cardlist};

        another_one.set_name("DOING");
        REQUIRE(another_one.get_name() != cardlist.get_name());

        CHECK(name_changed);
    }

    SECTION("Appending Items") {
        cardlist.container().append(Card{"Chores"});

        CHECK(appended_item);
    }

    SECTION("Appending Repeating Items") {
        Card chores_card{"Chores"};
        cardlist.container().append(chores_card);

        appended_item = false;

        cardlist.container().append(chores_card);

        CHECK_FALSE(appended_item);
    }

    SECTION("Removing Items") {
        auto card = cardlist.container().append(Card{"Chores"});
        auto card2 = cardlist.container().append(Card{"More Chores"});

        cardlist.container().remove(*card);
        CHECK(removed_item);
    }

    SECTION("Removing Non-Existing Items") {
        cardlist.container().remove(Card{"Chores"});
        CHECK_FALSE(removed_item);
    }

    SECTION("Reordering Items") {
        auto card = cardlist.container().append(Card{"Chores"});
        auto card2 = cardlist.container().append(Card{"More Chores"});

        cardlist.container().reorder(*card, *card2);
        CHECK(reordered_item);
    }

    SECTION("Reordering Non-Present Items") {
        auto card = cardlist.container().append(Card{"Chores"});

        cardlist.container().reorder(*card, Card{"nobody here"});
        CHECK_FALSE(reordered_item);
    }
}

TEST_CASE("Modification State", "[CardList]") {
    CardList cardlist{"TODO"};

    bool name_changed, appended_item, removed_item, reordered_item;
    name_changed = appended_item = removed_item = reordered_item = false;

    cardlist.signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });
    cardlist.container().signal_append().connect(
        [&appended_item](std::shared_ptr<Card>) { appended_item = true; });
    cardlist.container().signal_remove().connect(
        [&removed_item](std::shared_ptr<Card>) { removed_item = true; });
    cardlist.container().signal_reorder().connect(
        [&reordered_item](std::shared_ptr<Card>, std::shared_ptr<Card>,
                          ReorderingType) { reordered_item = true; });

    SECTION("Name Changing") {
        cardlist.set_name("DOING");

        CHECK(cardlist.modified());
    }

    SECTION("Appending Items") {
        REQUIRE_FALSE(cardlist.modified());

        cardlist.container().append(Card{"Chores"});

        CHECK(cardlist.modified());
    }

    SECTION("Removing Items") {
        auto card = cardlist.container().append(Card{"Chores"});
        auto card2 = cardlist.container().append(Card{"More Chores"});

        cardlist.container().modify(false);

        cardlist.container().remove(*card);
        CHECK(cardlist.modified());
    }

    SECTION("Removing Non-Existing Items") {
        cardlist.container().remove(Card{"Chores"});
        CHECK_FALSE(cardlist.modified());
    }

    SECTION("Reordering Items") {
        auto card = cardlist.container().append(Card{"Chores"});
        auto card2 = cardlist.container().append(Card{"More Chores"});

        cardlist.container().modify(false);

        cardlist.container().reorder(*card, *card2);
        CHECK(cardlist.modified());
    }

    SECTION("Reordering Non-Preset Items") {
        auto card = cardlist.container().append(Card{"Chores"});
        cardlist.container().modify(false);

        cardlist.container().reorder(*card, Card{"nobody here"});
        CHECK_FALSE(cardlist.modified());
    }
}