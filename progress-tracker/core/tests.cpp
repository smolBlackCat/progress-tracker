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
}

/**
 * It creates xml code of a progress tracker board
*/
TEST_CASE("Using TinyXML2") {
    const char* tasks1[] = {"Fix the computer", "Code project", "Do Math Assignment"};
    const char* tasks2[] = {"Read book", "Talk to someone", "Eat hamburger"};
    const char* lists_names[] = {"TODO", "Done"};
    tinyxml2::XMLDocument doc;

    tinyxml2::XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Computer Science Classes");
    board_element->SetAttribute("background", "(255,255,255,1)");
    doc.InsertEndChild(board_element);

    for (auto& list_name: lists_names) {
        // Create list
        tinyxml2::XMLElement* list_element = doc.NewElement("list");
        list_element->SetAttribute("name", list_name);

        for (auto& task: (list_name=="TODO"? tasks1:tasks2)) {
            // Create card
            tinyxml2::XMLElement* card_element = doc.NewElement("card");
            card_element->SetAttribute("name", task);
            list_element->InsertEndChild(card_element);
        }
        board_element->InsertEndChild(list_element);
    }

    tinyxml2::XMLPrinter printer;

    doc.Print(&printer);

    std::cout << printer.CStr() << std::endl;
}
