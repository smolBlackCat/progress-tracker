#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <filesystem>
#include <fstream>

#include "board.h"
#include "card.h"
#include "cardlist.h"
#include "item.h"

void create_dummy_file(const char* board_name, const char* board_background,
                       const char* filename) {
    const char* tasks1[] = {"Fix the computer", "Code project",
                            "Do Math Assignment"};
    const char* tasks2[] = {"Read book", "Talk to someone", "Eat hamburger"};
    const char* lists_names[] = {"TODO", "Done"};
    tinyxml2::XMLDocument doc;

    tinyxml2::XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", board_name);
    board_element->SetAttribute("background", board_background);
    doc.InsertEndChild(board_element);

    for (auto& list_name : lists_names) {
        // Create list
        tinyxml2::XMLElement* list_element = doc.NewElement("list");
        list_element->SetAttribute("name", list_name);

        for (auto& task : (list_name == "TODO" ? tasks1 : tasks2)) {
            // Create card
            tinyxml2::XMLElement* card_element = doc.NewElement("card");
            card_element->SetAttribute("name", task);
            list_element->InsertEndChild(card_element);
        }
        board_element->InsertEndChild(list_element);
    }

    doc.SaveFile(filename);
}

std::string* get_xml_from_file(const char* filename) {
    if (!std::filesystem::exists(filename)) return nullptr;

    std::string* file_content = new std::string{};
    std::fstream xml_file{filename};
    if (xml_file.is_open()) {
        std::string line;

        while (std::getline(xml_file, line)) {
            *file_content += line + "\n";
        }
        xml_file.close();
    } else {
        return nullptr;
    }

    return file_content;
}

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

    Card card{"Chores"}, card2{"Fix the computer"},
        card3{"Code for the cs course"};

    cardlist.add_card(card);
    cardlist.add_card(card2);
    cardlist.add_card(card3);
}

TEST_CASE("Removing cards from a CardList object") {
    CardList cardlist{"Todo"};

    Card card{"Chores"}, card2{"Fix the computer"},
        card3{"Code for the cs course"}, card4{"Fix the computer"};

    cardlist.add_card(card);
    cardlist.add_card(card2);
    cardlist.add_card(card3);
    cardlist.add_card(card4);

    CHECK(cardlist.remove_card(
        card));  // Remove the first card; should return true;
    CHECK_FALSE(cardlist.remove_card(
        card));  // try to remove the first card; return false;

    CHECK(cardlist.remove_card(card4));
    CHECK_FALSE(cardlist.remove_card(card4));
}

TEST_CASE("Basic Usage of a board") {
    // A domain_error exception is thrown when the background is not of
    // background type
    CHECK_THROWS(Board("Gabriel's Board", "not a background"));

    Board board{"Computer Science Stuff", "(255,255,255,1)"};  // valid;

    CHECK_FALSE(board.set_background("not a background"));
    CHECK(board.set_background("/home/moura/Pictures/cat.jpeg"));
    CHECK(board.set_background("(255,224,123,1)"));

    CardList todo_cardlist{"TODO"};
    std::string card_names1[] = {"OS assignment", "C++ Application"};
    for (auto& name : card_names1) {
        Card card{name};
        todo_cardlist.add_card(card);
    }
    CardList doing_cardlist{"DOING"};
    std::string card_names2[] = {"Progress-Tracker App",
                                 "Advanced Algebra Assignment"};
    for (auto& name : card_names2) {
        Card card{name};
        doing_cardlist.add_card(card);
    }

    CardList doing_cardlist2{"DOING"};
    std::string card_names3[] = {"Fixing", "Chores", "Homeschooling"};
    for (auto& name : card_names3) {
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

TEST_CASE("Creating boards from XML files") {
    if (!std::filesystem::exists("board_progress.xml")) {
        create_dummy_file("Computer Science Classes", "(255,255,255,1)",
                          "board_progress.xml");
    }
    REQUIRE(board_from_xml("non_existent_file.xml") == nullptr);

    Board* board = board_from_xml("board_progress.xml");
    REQUIRE(board != nullptr);

    std::string* file_content = get_xml_from_file("board_progress.xml");
    CHECK(board->xml_structure() == *file_content);

    if (!std::filesystem::exists("expected_to_fail.xml")) {
        create_dummy_file("Test went wrong", "not a background string",
                          "expected_to_fail.xml");
    }
    CHECK(board_from_xml("expected_to_fail.xml") == nullptr);
}
