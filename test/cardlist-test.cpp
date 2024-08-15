#define CATCH_CONFIG_MAIN

#include "cardlist.h"

#include <catch2/catch_test_macros.hpp>
#include <memory>

TEST_CASE("CardList operations", "[CardList]") {
    CardList cardList("MyCardList");

    SECTION("Add Card") {
        Card card1("Card1");
        auto addedCard1 = cardList.add_card(card1);

        REQUIRE(addedCard1 != nullptr);
        REQUIRE(*addedCard1 == card1);
        REQUIRE(cardList.get_card_vector().size() == 1);
        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Remove Card") {
        Card card1("Card1");
        Card card2("Card2");
        cardList.add_card(card1);
        cardList.add_card(card2);

        REQUIRE(cardList.remove_card(card1) == true);
        REQUIRE(cardList.get_card_vector().size() == 1);
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

        REQUIRE(cardList.get_card_vector().at(0) == cardPtr1);
        REQUIRE(cardList.get_card_vector().at(1) == cardPtr2);
        REQUIRE(cardList.get_card_vector().at(2) == cardPtr3);

        cardList.reorder_card(cardPtr3, cardPtr1);  // Move card3 after card1

        REQUIRE(cardList.get_card_vector().at(0) == cardPtr1);
        REQUIRE(cardList.get_card_vector().at(1) == cardPtr3);
        REQUIRE(cardList.get_card_vector().at(2) == cardPtr2);

        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Reorder Cards invalid arguments") {
        Card card1("Card1");
        Card card2("Card2");

        auto cardPtr1 = cardList.add_card(card1);

        REQUIRE_THROWS_AS(
            cardList.reorder_card(cardPtr1, std::make_shared<Card>(card2)),
            std::invalid_argument);
        REQUIRE_THROWS_AS(
            cardList.reorder_card(std::make_shared<Card>(card2), cardPtr1),
            std::invalid_argument);
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