#define CATCH_CONFIG_MAIN

#include "core/cardlist.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CardList operations", "[CardList]") {
    CardList cardList("MyCardList");

    SECTION("Add Card") {
        Card card1("Card1");
        auto addedCard1 = cardList.add_card(card1);

        REQUIRE(addedCard1 != nullptr);
        REQUIRE(*addedCard1 == card1);
        REQUIRE(cardList.get_cards().size() == 1);
        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Remove Card") {
        Card card1("Card1");
        Card card2("Card2");
        cardList.add_card(card1);
        cardList.add_card(card2);

        REQUIRE(cardList.remove_card(card1) == true);
        REQUIRE(cardList.get_cards().size() == 1);
        REQUIRE(cardList.remove_card(card1) ==
                false);  // Removing the same card should return false
        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Reorder Cards") {
        Card card1("Card1");
        Card card2("Card2");
        Card card3("Card3");

        auto cardPtr1 = cardList.add_card(card1);
        auto cardPtr2 = cardList.add_card(card2);
        auto cardPtr3 = cardList.add_card(card3);

        REQUIRE(*cardList.get_cards().at(0) == card1);
        REQUIRE(*cardList.get_cards().at(1) == card2);
        REQUIRE(*cardList.get_cards().at(2) == card3);

        cardList.reorder_card(card3, card1);  // Move card3 after card1

        REQUIRE(*cardList.get_cards().at(0) == card1);
        REQUIRE(*cardList.get_cards().at(1) == card3);
        REQUIRE(*cardList.get_cards().at(2) == card2);

        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Reorder Cards invalid arguments") {
        Card card1("Card1");
        Card card2("Card2");

        auto cardPtr1 = cardList.add_card(card1);
        cardList.set_modified(false);

        cardList.reorder_card(card1, card2);

        REQUIRE(!cardList.get_modified());
    }

    SECTION("Modified flag with internal card modification") {
        Card card1("Card1");

        auto cardPtr1 = cardList.add_card(card1);
        REQUIRE(cardList.get_modified() == true);

        cardList.get_modified();  // Reset the flag
        cardPtr1->set_modified(true);
        REQUIRE(cardList.get_modified() ==
                true);  // Should return true because internal card was modified
    }
}