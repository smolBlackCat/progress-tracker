#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "item.h"
#include "card.h"
#include "cardlist.h"
#include "board.h"

TEST_CASE("ID generation") {
    Item item{"name"};
    Item item2{"name"};

    // Items have same name but different IDs
    REQUIRE_FALSE(item == item2);
}

TEST_CASE("Setting a new name to an 'Item' object") {
    Item item{"Foo"};
    item.set_name("Bar");
    CHECK(item.get_name() == "Bar");
}

TEST_CASE("Setting a new name to an 'Card' object") {
    Card card{"To be Done"};
    card.set_name("To do");
    CHECK(card.get_name() == "To do");
}

TEST_CASE("Fetching the xml code from an 'Item' object") {
    Item item{"Chores"};

    std::string expected = "\t\t<item name=\"" + item.get_name() + "\">";

    CHECK(item.fetch_xml() == expected);
}

TEST_CASE("Fetching xml tags from Card objects") {
    Card card{"To do"};
    Card card2{"Doing"};
    Card card3{"Done"};

    CHECK(card.fetch_xml() == "\t\t<card name=\"" + card.get_name() + "\">");
    CHECK(card2.fetch_xml() == "\t\t<card name=\"" + card2.get_name() + "\">");
    CHECK(card3.fetch_xml() == "\t\t<card name=\"" + card3.get_name() + "\">");
}

TEST_CASE("Inheritance correctness") {
    Item* card = new Card("name");

    CHECK(card->fetch_xml() != "\t\t<item name=\"" + card->get_name() + "\">");

    delete card;
    card = nullptr;
}

TEST_CASE("Inserting cards into a cardlist") {
    CardList cardlist{"To do"};

    Card card1{"Chores"};
    Card card2{"Fix the computer"};
    Card card3{"Code for the cs course"};

    CHECK(cardlist.add_card(card1));
    CHECK(cardlist.add_card(card2));
    CHECK(cardlist.add_card(card3));
}

TEST_CASE("Fetching xml tags from CardList object") {
    CardList cardlist{"Todo"};

    Card card{"Chores"}, card2{"Fix the computer"}, card3{"Code for the cs course"};

    cardlist.add_card(card);
    cardlist.add_card(card2);
    cardlist.add_card(card3);

    std::string expected = "\t<list name=\"Todo\">\n"
        "\t\t<card name=\"Chores\">\n"
        "\t\t<card name=\"Fix the computer\">\n"
        "\t\t<card name=\"Code for the cs course\">\n"
        "\t</list>";

    CHECK(cardlist.fetch_xml() == expected);
}

TEST_CASE("Removing cards from a CardList object") {
    CardList cardlist{"Todo"};

    Card card{"Chores"}, card2{"Fix the computer"}, card3{"Code for the cs course"}, card4{"Fix the computer"};

    cardlist.add_card(card);
    cardlist.add_card(card2);
    cardlist.add_card(card3);
    cardlist.add_card(card4);

    CHECK(cardlist.remove_card(card));  // Remove the first card; should return true;
    CHECK_FALSE(cardlist.remove_card(card));  // try to remove the first card; return false;

    CHECK(cardlist.remove_card(card4));
    CHECK_FALSE(cardlist.remove_card(card4));

    std::string expected = "\t<list name=\"Todo\">\n"
        "\t\t<card name=\"Fix the computer\">\n"
        "\t\t<card name=\"Code for the cs course\">\n"
        "\t</list>";

    CHECK(cardlist.fetch_xml() == expected);
}

TEST_CASE("Basic Usage of a board") {
    // A domain_error exception is thrown when the background is not of background type
    CHECK_THROWS(Board("Gabriel's Board", "not a background"));

    Board board{"Computer Science Stuff", "(255,255,255,1)"};  //valid;

    CHECK_FALSE(board.set_background("not a background"));
    CHECK(board.set_background("/home/moura/Pictures/cat.jpeg"));
    CHECK(board.set_background("(255,224,123,1)"));

    CardList todo_cardlist{"TODO"};
    std::string card_names1[] = {"OS assignment", "C++ Application"};
    for (auto& name: card_names1) {
        Card card{name};
        todo_cardlist.add_card(card);
    }
    CardList doing_cardlist{"DOING"};
    std::string card_names2[] = {"Progress-Tracker App", "Advanced Algebra Assignment"};
    for (auto& name: card_names2) {
        Card card{name};
        doing_cardlist.add_card(card);
    }

    CardList doing_cardlist2{"DOING"};
    std::string card_names3[] = {"Fixing", "Chores", "Homeschooling"};
    for (auto& name: card_names3) {
        Card card{name};
        doing_cardlist2.add_card(card);
    }

    // Adding 
    CHECK(board.add_cardlist(todo_cardlist));
    CHECK(board.add_cardlist(todo_cardlist));

    CHECK(board.add_cardlist(doing_cardlist));
    CHECK(board.add_cardlist(doing_cardlist2));

    CHECK(board.remove_cardlist(doing_cardlist));

    // 'doing_cardlist' was already removed. must return false
    CHECK_FALSE(board.remove_cardlist(doing_cardlist));

    // Should not remove the duplicate list with different cards
    CHECK_FALSE(board.remove_cardlist(doing_cardlist));

    std::string expected = "<xml version=\"1.0\">\n"
        "<board name=\"Computer Science Stuff\" background=\"(255,224,123,1)\">\n"
        + todo_cardlist.fetch_xml() + "\n"
        + todo_cardlist.fetch_xml() + "\n"
        + doing_cardlist2.fetch_xml()
        + "\n</board>";
    
    CHECK(board.fetch_xml() == expected);
}

TEST_CASE("Creating model objects from xml code strings") {
    
}