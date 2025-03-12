#define CATCH_CONFIG_MAIN

#include <core/board.h>
#include <core/cardlist.h>
#include <core/exceptions.h>
#include <tinyxml2.h>

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <format>
#include <fstream>

TEST_CASE("Parsing valid XML Progress boards 1", "[Board]") {
    using namespace tinyxml2;
    using namespace std;

    XMLDocument doc;
    XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Progress Board");
    board_element->SetAttribute("background", "rgb(1,1,1)");
    doc.InsertFirstChild(board_element);
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    REQUIRE(backend.set_attribute("filepath", "test-board.xml"));
    Board board = backend.load();

    CHECK(board.get_name() == "Progress Board");
    CHECK(board.get_background() == "rgb(1,1,1)");
    CHECK(!board.get_modified());

    filesystem::remove("test-board.xml");
}

TEST_CASE("Parsing valid XML Progress boards 2", "[Board]") {
    using namespace tinyxml2;
    using namespace std;

    XMLDocument doc;
    XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Progress Board");
    board_element->SetAttribute("background", "rgb(1,1,1)");

    // Adding Valid Cardlists
    for (int i = 0; i < 10; i++) {
        XMLElement* list_element = board_element->InsertNewChildElement("list");
        list_element->SetAttribute("name",
                                   std::format("Cardlist {}", i).c_str());
        // Adding valid cards
        for (int j = 0; j < 10; j++) {
            XMLElement* card_element =
                list_element->InsertNewChildElement("card");
            card_element->SetAttribute("name",
                                       std::format("Card {}", j).c_str());
            card_element->SetAttribute("color", "rgb(23,12,45)");
            card_element->SetAttribute("due", "1969-01-01");
            card_element->SetAttribute("complete", false);

            // Adding valid tasks to the current card
            for (int k = 0; k < 10; k++) {
                XMLElement* task_element =
                    card_element->InsertNewChildElement("task");
                task_element->SetAttribute("name",
                                           std::format("Task {}", k).c_str());
                task_element->SetAttribute("done", true);
            }

            XMLElement* notes_element =
                card_element->InsertNewChildElement("notes");
            notes_element->SetText("I'm pretty loaded really yay");
        }
    }

    doc.InsertFirstChild(board_element);
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    REQUIRE(backend.set_attribute("filepath", "test-board.xml"));
    Board board = backend.load();

    CHECK(board.get_name() == "Progress Board");
    CHECK(board.get_background() == "rgb(1,1,1)");
    CHECK(!board.get_modified());

    filesystem::remove("test-board.xml");
}

TEST_CASE("Invalid XML Board: missing file path", "[Board]") {
    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath", "nonexistent-file.xml");
    CHECK_THROWS_AS(backend.load(), std::invalid_argument);
}

TEST_CASE("Invalid XML Board: file fails to load", "[Board]") {
    using namespace tinyxml2;
    XMLDocument doc;
    // Creating an invalid file that cannot be parsed
    std::ofstream outFile("corrupt-file.xml");
    outFile << "invalid XML content";
    outFile.close();

    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath", "corrupt-file.xml");
    CHECK_THROWS_AS(backend.load(), std::invalid_argument);

    std::filesystem::remove("corrupt-file.xml");
}

TEST_CASE("Invalid XML Board: missing <board> element", "[Board]") {
    using namespace tinyxml2;
    XMLDocument doc;
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath", "test-board.xml");
    CHECK_THROWS(backend.load());

    std::filesystem::remove("test-board.xml");
}

TEST_CASE(
    "Invalid XML Board: missing 'name' or 'background' attribute in <board>",
    "[Board]") {
    using namespace tinyxml2;
    XMLDocument doc;
    XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Progress Board");
    // Background attribute is missing
    doc.InsertFirstChild(board_element);
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath", "test-board.xml");
    CHECK_THROWS(backend.load());

    std::filesystem::remove("test-board.xml");
}

TEST_CASE("Invalid XML Board: missing 'name' attribute in <list>", "[Board]") {
    using namespace tinyxml2;
    XMLDocument doc;
    XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Progress Board");
    board_element->SetAttribute("background", "rgba(255,0,0,1)");

    XMLElement* list_element = board_element->InsertNewChildElement("list");
    // Name attribute for <list> is missing

    doc.InsertFirstChild(board_element);
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath", "test-board.xml");
    Board board = backend.load();
    CHECK_THROWS(board.load());

    std::filesystem::remove("test-board.xml");
}

TEST_CASE("Invalid XML Board: missing 'name' attribute in <card>", "[Board]") {
    using namespace tinyxml2;
    XMLDocument doc;
    XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Progress Board");
    board_element->SetAttribute("background", "rgba(255,0,0,1)");

    XMLElement* list_element = board_element->InsertNewChildElement("list");
    list_element->SetAttribute("name", "List 1");

    XMLElement* card_element = list_element->InsertNewChildElement("card");
    card_element->SetAttribute("color", "rgb(23,12,45)");
    // Name attribute for <card> is missing

    doc.InsertFirstChild(board_element);
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath", "test-board.xml");
    Board board = backend.load();
    CHECK_THROWS(board.load());

    std::filesystem::remove("test-board.xml");
}

TEST_CASE("Invalid XML Board: malformed due date in <card>", "[Board]") {
    using namespace tinyxml2;
    XMLDocument doc;
    XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Progress Board");
    board_element->SetAttribute("background", "rgba(255,0,0,1)");

    XMLElement* list_element = board_element->InsertNewChildElement("list");
    list_element->SetAttribute("name", "List 1");

    XMLElement* card_element = list_element->InsertNewChildElement("card");
    card_element->SetAttribute("name", "Card 1");
    card_element->SetAttribute("due", "invalid-date");  // Malformed date

    doc.InsertFirstChild(board_element);
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath", "test-board.xml");

    Board board = backend.load();
    CHECK_THROWS(board.load());

    std::filesystem::remove("test-board.xml");
}

TEST_CASE("Invalid XML Board: missing 'name' attribute in <task>", "[Board]") {
    using namespace tinyxml2;
    XMLDocument doc;
    XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Progress Board");
    board_element->SetAttribute("background", "rgba(255,0,0,1)");

    XMLElement* list_element = board_element->InsertNewChildElement("list");
    list_element->SetAttribute("name", "List 1");

    XMLElement* card_element = list_element->InsertNewChildElement("card");
    card_element->SetAttribute("name", "Card 1");

    XMLElement* task_element = card_element->InsertNewChildElement("task");
    task_element->SetAttribute("done", true);  // Missing 'name' attribute

    doc.InsertFirstChild(board_element);
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    backend.set_attribute("filepath", "test-board.xml");
    Board board = backend.load();
    CHECK_THROWS(board.load());

    std::filesystem::remove("test-board.xml");
}

TEST_CASE("Board late loading", "[Board]") {
    using namespace tinyxml2;
    using namespace std;

    XMLDocument doc;
    XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", "Progress Board");
    board_element->SetAttribute("background", "rgb(1,1,1)");

    // Adding Valid Cardlists
    for (int i = 0; i < 10; i++) {
        XMLElement* list_element = board_element->InsertNewChildElement("list");
        list_element->SetAttribute("name",
                                   std::format("Cardlist {}", i).c_str());
        // Adding valid cards
        for (int j = 0; j < 10; j++) {
            XMLElement* card_element =
                list_element->InsertNewChildElement("card");
            card_element->SetAttribute("name",
                                       std::format("Card {}", j).c_str());
            card_element->SetAttribute("color", "rgb(23,12,45)");
            card_element->SetAttribute("due", "1969-01-01");
            card_element->SetAttribute("complete", false);

            // Adding valid tasks to the current card
            for (int k = 0; k < 10; k++) {
                XMLElement* task_element =
                    card_element->InsertNewChildElement("task");
                task_element->SetAttribute("name",
                                           std::format("Task {}", k).c_str());
                task_element->SetAttribute("done", true);
            }

            XMLElement* notes_element =
                card_element->InsertNewChildElement("notes");
            notes_element->SetText("I'm pretty loaded really yay");
        }
    }

    doc.InsertFirstChild(board_element);
    doc.SaveFile("test-board.xml");

    BoardBackend backend{BackendType::LOCAL};
    REQUIRE(backend.set_attribute("filepath", "test-board.xml"));
    Board board = backend.load();

    // Basic properties must be available
    CHECK(board.get_name() == "Progress Board");
    CHECK(board.get_background() == "rgb(1,1,1)");
    CHECK(!board.get_modified());

    CHECK(!board.is_loaded());
    CHECK(board.get_cardlists().empty());

    board.load();

    CHECK(board.is_loaded());
    CHECK(board.get_cardlists().size() == 10);

    filesystem::remove("test-board.xml");
}

TEST_CASE("Adding Cardlists to a Board", "[Board]") {
    BoardBackend backend{BackendType::LOCAL};
    Board board = backend.create("Progress Board", "rgb(1,2,3)");

    REQUIRE(!board.get_modified());

    // Cardlists can be superficially the same, but there must be no repeated
    // ids
    CardList cardlist1{"TODO"};
    CardList cardlist2{"TODO"};

    auto added_cardlist = board.add(cardlist1);
    auto added_cardlist2 = board.add(cardlist2);

    REQUIRE(added_cardlist);
    REQUIRE(added_cardlist2);

    // We're trying to add the exact same cardlist again. Don't and return
    // nullptr
    CHECK(!board.add(cardlist1));

    CHECK(board.get_modified());
    CHECK(board.get_cardlists().size() == 2);
    CHECK(*board.get_cardlists()[0] == cardlist1);
    CHECK(*board.get_cardlists()[1] == cardlist2);
}

TEST_CASE("Removing cardlists of a Board", "[Board]") {
    BoardBackend backend{BackendType::LOCAL};
    Board board = backend.create("Progress Board", "rgb(1,2,3)");

    CardList cardlist1{"Something else to be done"};

    board.add(cardlist1);
    board.set_modified(false);

    CHECK(board.remove(cardlist1));
    CHECK(board.get_modified());

    board.set_modified(false);

    CHECK(!board.remove(cardlist1));
    CHECK(!board.get_modified());
}

TEST_CASE("Reordering Cardlists", "Board") {
    Board board =
        BoardBackend{BackendType::LOCAL}.create("Progress", "file.png");

    auto cardlist1 = board.add(CardList{"CardList 1"});
    auto cardlist2 = board.add(CardList{"CardList 2"});
    auto cardlist3 = board.add(CardList{"CardList 3"});

    auto& cardlists = board.get_cardlists();

    board.set_modified(false);

    SECTION("Reordering using the same object references") {
        board.reorder(*cardlist1, *cardlist1);

        // Because we're trying to reorder the same thing, no reordering is
        // done, thus no modification either
        CHECK(*cardlists[0] == *cardlist1);
        CHECK(*cardlists[1] == *cardlist2);
        CHECK(*cardlists[2] == *cardlist3);
        CHECK(!board.get_modified());
    }

    board.set_modified(false);

    SECTION("Reordering with one absent cardlist") {
        board.reorder(*cardlist1, CardList{"nobody here"});

        // Trying to reorder a cardlist with an absent one does nothing exactly
        // since there's no other card to complete the operation
        CHECK(*cardlists[0] == *cardlist1);
        CHECK(*cardlists[1] == *cardlist2);
        CHECK(*cardlists[2] == *cardlist3);
        CHECK(!board.get_modified());
    }

    board.set_modified(false);

    SECTION("Reordering where sibling and next are already sibling and next") {
        board.reorder(*cardlist3, *cardlist2);

        // Because cardlist3 is already next to cardlist2, They'll just exchange
        // places
        CHECK(*cardlists[0] == *cardlist1);
        CHECK(*cardlists[1] == *cardlist3);
        CHECK(*cardlists[2] == *cardlist2);
        CHECK(board.get_modified());
    }

    board.set_modified(false);

    SECTION("Reordering case 1") {
        board.reorder(*cardlist1, *cardlist3);

        // Cardlist1 will be placed after cardlist 3, ths cardlist1 will be the
        // last element and a modification is registered
        CHECK(cardlists[0] == cardlist2);
        CHECK(cardlists[1] == cardlist3);
        CHECK(cardlists[2] == cardlist1);
        CHECK(board.get_modified());
    }

    board.set_modified(false);

    SECTION("Reordering case 2") {
        board.reorder(*cardlist3, *cardlist1);

        // Cardlist 3 will be placed in the Cardlist 1 position, thus being the
        // first item
        CHECK(*cardlists[0] == *cardlist3);
        CHECK(*cardlists[1] == *cardlist1);
        CHECK(*cardlists[2] == *cardlist2);
        CHECK(board.get_modified());
    }
}

TEST_CASE("Saving new boards") {
    using namespace tinyxml2;
    Board board =
        BoardBackend{BackendType::LOCAL}.create("Progress", "file.png");
    board.backend.set_attribute("filepath", "new-board.xml");

    CardList cardlist{"Things to do"};
    Card card{"Computer Science", Color{123, 456, 123, 1}};
    cardlist.add(card);
    board.add(cardlist);

    board.save();

    REQUIRE(!board.get_modified());

    XMLDocument doc;
    doc.LoadFile("new-board.xml");

    XMLElement* board_element = doc.FirstChildElement("board");
    CHECK(std::string{board_element->FindAttribute("name")->Value()} ==
          "Progress");

    // file.png does not exist. So the default one is set
    CHECK(std::string{board_element->FindAttribute("background")->Value()} ==
          Board::BACKGROUND_DEFAULT);

    XMLElement* list_element = board_element->FirstChildElement("list");
    CHECK(list_element->FindAttribute("name")->Value() ==
          std::string{"Things to do"});

    XMLElement* card_element = list_element->FirstChildElement("card");
    CHECK(card_element->FindAttribute("name")->Value() ==
          std::string{"Computer Science"});

    std::filesystem::remove("new-board.xml");
}