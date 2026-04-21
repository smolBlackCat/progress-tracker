#include "core/cardlist.h"

#define CATCH_CONFIG_MAIN

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <format>

TEST_CASE("CardList Instantiation", "[CardList]") {
    auto cardlist = CardList::create("TODO");

    REQUIRE_FALSE(cardlist->modified());

    CHECK(cardlist->get_name() == "TODO");
    CHECK(cardlist->container().get_data().size() == 0);
}

TEST_CASE("Signal Emission", "[CardList]") {
    auto cardlist = CardList::create("TODO");

    bool name_changed, appended_item, removed_item, reordered_item;
    name_changed = appended_item = removed_item = reordered_item = false;

    cardlist->signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });
    cardlist->container().signal_append().connect(
        [&appended_item](std::shared_ptr<Card>) { appended_item = true; });
    cardlist->container().signal_remove().connect(
        [&removed_item](std::shared_ptr<Card>) { removed_item = true; });
    cardlist->container().signal_reorder().connect(
        [&reordered_item](std::shared_ptr<Card>, std::shared_ptr<Card>,
                          ReorderingType r) {
            if (r != ReorderingType::INVALID) {
                reordered_item = true;
            }
        });

    SECTION("Name Changing") {
        cardlist->set_name("DOING");

        CHECK(name_changed);
    }

    SECTION("Appending Items") {
        auto card = Card::create("Chores");
        cardlist->container().append(card);

        CHECK(appended_item);
    }

    SECTION("Appending Repeating Items") {
        auto card = Card::create("Chores");
        cardlist->container().append(card);

        appended_item = false;

        cardlist->container().append(card);

        CHECK_FALSE(appended_item);
    }

    SECTION("Removing Items") {
        auto card = Card::create("Chores");
        auto card2 = Card::create("More Chores");

        cardlist->container().append(card);
        cardlist->container().append(card2);

        cardlist->container().remove(card);
        CHECK(removed_item);
    }

    SECTION("Removing Non-Existing Items") {
        auto card = Card::create("Chores");
        cardlist->container().remove(card);
        CHECK_FALSE(removed_item);
    }

    SECTION("Reordering Items") {
        auto card = Card::create("Chores");
        auto card2 = Card::create("More Chores");

        cardlist->container().append(card);
        cardlist->container().append(card2);

        cardlist->container().reorder_after(card, card2);
        CHECK(reordered_item);
    }

    SECTION("Reordering Non-Present Items") {
        auto card = Card::create("Chores");
        auto n_card = Card::create("Nothing here");
        cardlist->container().append(card);

        cardlist->container().reorder_after(card, n_card);
        CHECK_FALSE(reordered_item);
    }

    SECTION("Inserting items at beginning") {
        const int n_cards = 5;
        std::array<std::shared_ptr<Card>, n_cards> ptrs;
        for (int i = 0; i < n_cards; i++) {
            auto card = Card::create(std::format("Card {}", i));
            cardlist->container().append(card);
            ptrs[i] = card;
        }

        REQUIRE(cardlist->container().size() == 5);
        REQUIRE(cardlist->container().modified());

        auto card = Card::create("New Card");
        cardlist->container().insert_before(card, ptrs[0]);

        CHECK((cardlist->container().get_data()[0]->get_name()) ==
              card->get_name());
    }

    SECTION("Inserting items at the end") {
        const int n_cards = 5;
        std::array<std::shared_ptr<Card>, n_cards> ptrs;
        for (int i = 0; i < n_cards; i++) {
            auto card = Card::create(std::format("Card {}", i));
            cardlist->container().append(card);
            ptrs[i] = card;
        }

        REQUIRE(cardlist->container().size() == 5);
        REQUIRE(cardlist->container().modified());

        auto card = Card::create("New Card");
        cardlist->container().insert_before(card, ptrs[n_cards - 1]);

        CHECK((cardlist->container().get_data()[n_cards - 1])->get_name() ==
              card->get_name());
    }

    SECTION("Inserting items at middle") {
        const int n_cards = 5;
        std::array<std::shared_ptr<Card>, n_cards> ptrs;
        for (int i = 0; i < n_cards; i++) {
            auto card = Card::create(std::format("Card {}", i));
            cardlist->container().append(card);
            ptrs[i] = card;
        }

        REQUIRE(cardlist->container().size() == 5);
        REQUIRE(cardlist->container().modified());

        auto card = Card::create("New Card");
        // We're roughly adding it in the middle;
        cardlist->container().insert_before(card, ptrs[(n_cards + 1) / 2]);

        CHECK(cardlist->container().get_data()[(n_cards + 1) / 2] == card);

        auto another_card = Card::create("Another one");
        // We're roughly adding it in the middle;
        cardlist->container().insert_after(another_card,
                                           ptrs[((n_cards + 1) / 2)]);
        CHECK((cardlist->container().get_data()[((n_cards + 1) / 2) + 2])
                  ->get_name() == another_card->get_name());
    }
}

TEST_CASE("Modification State", "[CardList]") {
    auto cardlist = CardList::create("TODO");

    bool name_changed, appended_item, removed_item, reordered_item;
    name_changed = appended_item = removed_item = reordered_item = false;

    cardlist->signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });
    cardlist->container().signal_append().connect(
        [&appended_item](std::shared_ptr<Card>) { appended_item = true; });
    cardlist->container().signal_remove().connect(
        [&removed_item](std::shared_ptr<Card>) { removed_item = true; });
    cardlist->container().signal_reorder().connect(
        [&reordered_item](std::shared_ptr<Card>, std::shared_ptr<Card>,
                          ReorderingType) { reordered_item = true; });

    SECTION("Name Changing") {
        cardlist->set_name("DOING");

        CHECK(cardlist->modified());
    }

    SECTION("Appending Items") {
        REQUIRE_FALSE(cardlist->modified());
        auto card = Card::create("Chores");
        cardlist->container().append(card);

        CHECK(cardlist->modified());
    }

    SECTION("Removing Items") {
        auto card = Card::create("Chores");
        auto card2 = Card::create("More Chores");

        cardlist->container().append(card);

        cardlist->container().modify(false);

        cardlist->container().remove(card);
        CHECK(cardlist->modified());
    }

    SECTION("Removing Non-Existing Items") {
        auto card = Card::create("Chores");
        cardlist->container().remove(card);
        CHECK_FALSE(cardlist->modified());
    }

    SECTION("Reordering Items") {
        auto card = Card::create("Chores");
        auto card2 = Card::create("More Chores");

        cardlist->container().append(card);
        cardlist->container().append(card2);

        cardlist->container().modify(false);

        cardlist->container().reorder_after(card, card2);
        CHECK(cardlist->modified());
    }

    SECTION("Reordering Non-Preset Items") {
        auto card = Card::create("Chores");
        cardlist->container().append(card);
        cardlist->container().modify(false);

        auto nobody = Card::create("nobody here");
        cardlist->container().reorder_after(card, nobody);
        CHECK_FALSE(cardlist->modified());
    }
}
