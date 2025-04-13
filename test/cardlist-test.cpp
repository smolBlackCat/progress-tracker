#define CATCH_CONFIG_MAIN

#include <core/cardlist.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CardList operations", "[CardList]") {
    CardList cardList("MyCardList");

    SECTION("Add Card") {
        Card card1("Card1");
        auto addedCard1 = cardList.container().append(card1);

        REQUIRE(addedCard1 != nullptr);
        REQUIRE(*addedCard1 == card1);
        REQUIRE(cardList.container().get_data().size() == 1);
        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Remove Card") {
        Card card1("Card1");
        Card card2("Card2");
        cardList.container().append(card1);
        cardList.container().append(card2);

        cardList.container().remove(card1);
        REQUIRE(cardList.container().get_data().size() == 1);

        cardList.container().remove(card1);
        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Reorder Cards down-up") {
        Card card1("Card1");
        Card card2("Card2");
        Card card3("Card3");

        auto cardPtr1 = cardList.container().append(card1);
        auto cardPtr2 = cardList.container().append(card2);
        auto cardPtr3 = cardList.container().append(card3);

        REQUIRE(*cardList.container().get_data().at(0) == card1);
        REQUIRE(*cardList.container().get_data().at(1) == card2);
        REQUIRE(*cardList.container().get_data().at(2) == card3);

        cardList.container().reorder(card3,
                                     card1);  // Place card3 in card1's position

        REQUIRE(*cardList.container().get_data().at(0) == card3);
        REQUIRE(*cardList.container().get_data().at(1) == card1);
        REQUIRE(*cardList.container().get_data().at(2) == card2);

        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Reorder Cards up-down") {
        Card card1("Card1");
        Card card2("Card2");
        Card card3("Card3");

        auto cardPtr1 = cardList.container().append(card1);
        auto cardPtr2 = cardList.container().append(card2);
        auto cardPtr3 = cardList.container().append(card3);

        REQUIRE(*cardList.container().get_data().at(0) == card1);
        REQUIRE(*cardList.container().get_data().at(1) == card2);
        REQUIRE(*cardList.container().get_data().at(2) == card3);

        cardList.container().reorder(card1,
                                     card3);  // Place card3 in card1's position

        REQUIRE(*cardList.container().get_data().at(0) == card2);
        REQUIRE(*cardList.container().get_data().at(1) == card3);
        REQUIRE(*cardList.container().get_data().at(2) == card1);

        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Reordering Cards that are already in order") {
        Card card1("Card1");
        Card card2("Card2");
        Card card3("Card3");

        auto cardPtr1 = cardList.container().append(card1);
        auto cardPtr2 = cardList.container().append(card2);
        auto cardPtr3 = cardList.container().append(card3);

        REQUIRE(*cardList.container().get_data().at(0) == card1);
        REQUIRE(*cardList.container().get_data().at(1) == card2);
        REQUIRE(*cardList.container().get_data().at(2) == card3);

        // Cards will simply exchange places
        cardList.container().reorder(card2, card1);

        REQUIRE(*cardList.container().get_data().at(0) == card2);
        REQUIRE(*cardList.container().get_data().at(1) == card1);
        REQUIRE(*cardList.container().get_data().at(2) == card3);

        REQUIRE(cardList.get_modified() == true);
    }

    SECTION("Reorder Cards invalid arguments") {
        Card card1("Card1");
        Card card2("Card2");

        auto cardPtr1 = cardList.container().append(card1);
        cardList.set_modified(false);
        cardList.container().set_modified(false);

        cardList.container().reorder(card1, card2);

        REQUIRE(!cardList.get_modified());
    }

    SECTION("Modified flag with internal card modification") {
        Card card1("Card1");

        auto cardPtr1 = cardList.container().append(card1);
        REQUIRE(cardList.get_modified());

        cardList.get_modified();  // Reset the flag
        cardPtr1->set_modified(true);
        REQUIRE(cardList.get_modified() ==
                true);  // Should return true because internal card was modified
    }
}