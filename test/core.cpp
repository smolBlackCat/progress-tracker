#define CATCH_CONFIG_MAIN
#include <tinyxml2.h>

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <type_traits>

#include "board.h"
#include "card.h"
#include "cardlist.h"
#include "item.h"
#include "task.h"

std::string xml_structure(Board& board) {
    auto doc = std::make_unique<tinyxml2::XMLDocument>();

    tinyxml2::XMLElement* board_element = doc->NewElement("board");
    board_element->SetAttribute("name", board.get_name().c_str());
    board_element->SetAttribute("background", board.get_background().c_str());
    doc->InsertEndChild(board_element);

    for (auto& cardlist : board.get_cardlist_vector()) {
        tinyxml2::XMLElement* list_element = doc->NewElement("list");
        list_element->SetAttribute("name", cardlist->get_name().c_str());

        for (auto& card : cardlist->get_card_vector()) {
            tinyxml2::XMLElement* card_element = doc->NewElement("card");
            card_element->SetAttribute("name", card->get_name().c_str());
            for (auto& task : card->get_tasks()) {
                tinyxml2::XMLElement* task_element = doc->NewElement("task");
                task_element->SetAttribute("name", task->get_name().c_str());
                task_element->SetAttribute("done", task->get_done());

                card_element->InsertEndChild(task_element);
            }
            tinyxml2::XMLElement* notes_element = doc->NewElement("notes");
            notes_element->SetText(card->get_notes().c_str());
            card_element->InsertEndChild(notes_element);
            list_element->InsertEndChild(card_element);
        }
        board_element->InsertEndChild(list_element);
    }

    tinyxml2::XMLPrinter printer;
    doc->Print(&printer);

    return printer.CStr();
}

void create_dummy_file(const char* board_name, const char* board_background,
                       const char* filename) {
    const char* cards1[] = {"Fix the computer", "Code project",
                            "Do Math Assignment"};
    const char* cards2[] = {"Read book", "Talk to someone", "Eat hamburger"};
    const char* cardlists[] = {"TODO", "Done"};
    tinyxml2::XMLDocument doc;

    tinyxml2::XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", board_name);
    board_element->SetAttribute("background", board_background);
    doc.InsertEndChild(board_element);

    for (auto& cardlist : cardlists) {
        // Create list
        tinyxml2::XMLElement* cardlist_element = doc.NewElement("list");
        cardlist_element->SetAttribute("name", cardlist);

        for (auto& card :
             (std::string{cardlist} == std::string{"TODO"} ? cards1 : cards2)) {
            // Create card
            tinyxml2::XMLElement* card_element = doc.NewElement("card");
            card_element->SetAttribute("name", card);

            // Notes
            tinyxml2::XMLElement* notes_element = doc.NewElement("notes");
            notes_element->SetText("");
            card_element->InsertEndChild(notes_element);
            cardlist_element->InsertEndChild(card_element);
        }
        board_element->InsertEndChild(cardlist_element);
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

TEST_CASE("Creating Boards and setting different backgrounds") {
    Board board0{"Another Important Stuff", "invalid-background"};
    CHECK(board0.get_background() == Board::BACKGROUND_DEFAULT);

    Board board1{"Computer Science Stuff", "rgba(255,255,255,1)"};
    CHECK(BackgroundType::INVALID == board1.set_background("not a background"));
    CHECK(BackgroundType::IMAGE ==
          board1.set_background("/home/moura/Pictures/cat-better.png"));
    CHECK(BackgroundType::COLOR ==
          board1.set_background("rgba(255,224,123,1)"));
}

TEST_CASE("Modifying Board objects") {
    Board board1{"Computer Science Stuff", "rgba(255,255,255,1)"};
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

    CHECK(board1.add_cardlist(todo_cardlist));
    CHECK(board1.add_cardlist(todo_cardlist));
    CHECK(board1.add_cardlist(doing_cardlist));
    CHECK(board1.add_cardlist(doing_cardlist2));

    CHECK(board1.remove_cardlist(doing_cardlist));
    CHECK_FALSE(board1.remove_cardlist(doing_cardlist));
}

TEST_CASE(
    "Creating Boards from XML files: Creating board from inexistent files") {
    REQUIRE_THROWS(Board("non-existent-file.xml"));
}

TEST_CASE(
    "Creating boards from XML files: Creating a board from an existing file") {
    if (!std::filesystem::exists("board_progress.xml")) {
        create_dummy_file("Computer Science Classes", "rgba(255,255,255,1)",
                          "board_progress.xml");
    }
    REQUIRE_NOTHROW(Board{"board_progress.xml"});

    Board board{"board_progress.xml"};

    std::string* file_content = get_xml_from_file("board_progress.xml");
    CHECK(xml_structure(board) == *file_content);

    if (!std::filesystem::exists("board-with-invalid-background.xml")) {
        create_dummy_file("Test went wrong", "not a background string",
                          "board-with-invalid-background.xml");
    }
    REQUIRE_NOTHROW(Board("board-with-invalid-background.xml"));
}

void write_to(const std::string& filename, const std::string& data) {
    std::ofstream output_file{filename};

    if (output_file.is_open()) {
        output_file << data;
        output_file.close();
    } else {
        std::cerr << "It was not possible to create file " << filename
                  << std::endl;
    }
}

TEST_CASE("Triggering Progress Board XML parsing errors") {
    CHECK_THROWS(Board{"board-does-not-exist.xml"});

    std::string invalid_boards_xml[] = {
        // Board doesn't have a name attribute
        "<board>"
        "background=\"rgb(4,31,33)\">"
        "<list name=\"To Learn\">"
        "<card name=\"Agile Software Development\"/>"
        "<card name=\"Requirements Engineering\"/>"
        "<card name=\"System Modeling\"/>"
        "<card name=\"Case Studies\"/>"
        "</list>"
        "<list name=\"Studying\">"
        "<card name=\"Software Processes\"/>"
        "<card name=\"Coping with change\"/>"
        "<card name=\"Process Improvement\"/>"
        "</list>"
        "<list name=\"Studied\">"
        "<card name=\"Introduction to Software Engineering\"/>"
        "<card name=\"Software Engineering ethics\"/>"
        "<card name=\"Professional Software Development\"/>"
        "<card name=\"Integration and configuration\"/>"
        "</list>"
        "<list name=\"To Review\">"
        "<card name=\"Incremental Development\"/>"
        "<card name=\"Process Activities\"/>"
        "</list>"
        "</board>",

        // Board has an empty name
        "<board name=\"\" "
        "background=\"rgb(4,31,33)\">"
        "<list name=\"To Learn\">"
        "<card name=\"Agile Software Development\"/>"
        "<card name=\"Requirements Engineering\"/>"
        "<card name=\"System Modeling\"/>"
        "<card name=\"Case Studies\"/>"
        "</list>"
        "<list name=\"Studying\">"
        "<card name=\"Software Processes\"/>"
        "<card name=\"Coping with change\"/>"
        "<card name=\"Process Improvement\"/>"
        "</list>"
        "<list name=\"Studied\">"
        "<card name=\"Introduction to Software Engineering\"/>"
        "<card name=\"Software Engineering ethics\"/>"
        "<card name=\"Professional Software Development\"/>"
        "<card name=\"Integration and configuration\"/>"
        "</list>"
        "<list name=\"To Review\">"
        "<card name=\"Incremental Development\"/>"
        "<card name=\"Process Activities\"/>"
        "</list>"
        "</board>",

        // Board has ill-formed list elements
        "<board name=\"Software Engineering Subject\" "
        "background=\"rgb(4,31,33)\">"
        "<list name=\"To Learn\">"
        "<card name=\"Agile Software Development\"/>"
        "<card name=\"Requirements Engineering\"/>"
        "<card name=\"System Modeling\"/>"
        "<card name=\"Case Studies\"/>"
        "</list>"
        "<list>"  // list element must have a name attribute
        "<card name=\"Software Processes\"/>"
        "<card name=\"Coping with change\"/>"
        "<card name=\"Process Improvement\"/>"
        "</list>"
        "<list name=\"Studied\">"
        "<card name=\"Introduction to Software Engineering\"/>"
        "<card name=\"Software Engineering ethics\"/>"
        "<card name=\"Professional Software Development\"/>"
        "<card name=\"Integration and configuration\"/>"
        "</list>"
        "<list name=\"To Review\">"
        "<card name=\"Incremental Development\"/>"
        "<card name=\"Process Activities\"/>"
        "</list>"
        "</board>",

        // Board have an ill-formed card element
        "<board name=\"Software Engineering Subject\" "
        "background=\"rgb(4,31,33)\">"
        "<list name=\"To Learn\">"
        "<card name=\"Agile Software Development\"/>"
        "<card name=\"Requirements Engineering\"/>"
        "<card name=\"System Modeling\"/>"
        "<card name=\"Case Studies\"/>"
        "</list>"
        "<list name=\"Studying\">"
        "<card/>"  // Culprit card
        "<card name=\"Coping with change\"/>"
        "<card name=\"Process Improvement\"/>"
        "</list>"
        "<list name=\"Studied\">"
        "<card name=\"Introduction to Software Engineering\"/>"
        "<card name=\"Software Engineering ethics\"/>"
        "<card name=\"Professional Software Development\"/>"
        "<card name=\"Integration and configuration\"/>"
        "</list>"
        "<list name=\"To Review\">"
        "<card name=\"Incremental Development\"/>"
        "<card name=\"Process Activities\"/>"
        "</list>"
        "</board>",

        // Board is not defined at all
        "<list name=\"To Learn\">"
        "<card name=\"Agile Software Development\"/>"
        "<card name=\"Requirements Engineering\"/>"
        "<card name=\"System Modeling\"/>"
        "<card name=\"Case Studies\"/>"
        "</list>"
        "<list name=\"Studying\">"
        "<card name=\"Software Processes\"/>"
        "<card name=\"Coping with change\"/>"
        "<card name=\"Process Improvement\"/>"
        "</list>"
        "<list name=\"Studied\">"
        "<card name=\"Introduction to Software Engineering\"/>"
        "<card name=\"Software Engineering ethics\"/>"
        "<card name=\"Professional Software Development\"/>"
        "<card name=\"Integration and configuration\"/>"
        "</list>"
        "<list name=\"To Review\">"
        "<card name=\"Incremental Development\"/>"
        "<card name=\"Process Activities\"/>"
        "</list>"};

    for (int i = 0; i < 5; i++) {
        std::string filename = std::format("board-{}.xml", i);
        write_to(filename, invalid_boards_xml[i]);
        CHECK_THROWS(Board{filename});
        std::filesystem::remove("./" + filename);
    }
}

TEST_CASE("Saving boards: Successful attempt") {
    Board board{"Progress-tracker", "rgba(0,0,170,1)"};

    board.set_filepath("./progress-tracker-board.xml");
    CHECK(board.save_as_xml());

    std::string xml_structure1 = xml_structure(board);

    auto cardlist = CardList{"Fatal"};
    board.add_cardlist(cardlist);
    CHECK(board.save_as_xml());

    std::string xml_structure2 = xml_structure(board);

    CHECK_FALSE(xml_structure1 == xml_structure2);

    std::filesystem::remove("./progress-tracker-board.xml");
}

TEST_CASE("Saving Boards: Reliability") {
    std::string base_dir = "./boards/";
    std::string filepath = base_dir + "marki.xml";

    Board board{"Mark I", "rgb(0,0,0)"};

    REQUIRE(board.set_filepath(filepath) == true);
    std::filesystem::remove(base_dir);
    REQUIRE(board.save_as_xml());
    REQUIRE(std::filesystem::exists(filepath));

    std::filesystem::remove(filepath);
    std::filesystem::remove(base_dir);

    REQUIRE_FALSE(board.save_as_xml(false));
}

template <typename T>
    requires std::is_base_of_v<Item, T>
bool order_match(std::vector<std::shared_ptr<T>> order,
                 std::string* order_to_match) {
    for (size_t i = 0; i < order.size(); i++) {
        if (order[i]->get_name() != order_to_match[i]) {
            return false;
        }
    }
    return true;
}

TEST_CASE("Cards reordering within a CardList") {
    CardList cardlist{"Fatal"};

    Card card1{""}, card2{""}, card3{""};
    card1 = Card{"Programming"};
    card2 = Card{"Planning"};
    card3 = Card{"Productivity"};

    auto card1_refptr = cardlist.add_card(card1);
    auto card2_refptr = cardlist.add_card(card2);
    auto card3_refptr = cardlist.add_card(card3);

    // Order is: Programming, Planning, Productivity

    cardlist.reorder_card(card1_refptr, card2_refptr);
    // New expected order is: Planning, Programming, Productivity
    std::string expected_order[] = {"Planning", "Programming", "Productivity"};
    CHECK(order_match(cardlist.get_card_vector(), expected_order));

    cardlist.reorder_card(card2_refptr, card3_refptr);
    // New expected order: Programming, Productivity, Planning
    std::string expected_order1[] = {"Programming", "Productivity", "Planning"};
    CHECK(order_match(cardlist.get_card_vector(), expected_order1));
}

TEST_CASE("Checking for board modification: Adding cards to a cardlist") {
    create_dummy_file("TestBoard", "rgba(0,0,0,1)", "test-board.xml");

    Board test_board{"test-board.xml"};

    test_board.get_cardlist_vector()[0]->add_card(Card{"Fatal"});

    CHECK(test_board.get_modified());
    std::filesystem::remove("./test-board.xml");
}

TEST_CASE("Checking for board modification: Adding cardlists to a board") {
    create_dummy_file("TestBoard", "rgba(0,0,0,1)", "test-board.xml");

    Board test_board{"test-board.xml"};
    test_board.add_cardlist(CardList{"New Cardlist"});

    CHECK(test_board.get_modified());
    std::filesystem::remove("./test-board.xml");
}

TEST_CASE("Checking for board modification: Reordering Items") {
    create_dummy_file("TestBoard", "rgba(0,0,0,1)", "test-board.xml");

    Board test_board{"test-board.xml"};

    // Reordering Cardlists
    auto cardlist1 = test_board.get_cardlist_vector()[0];
    auto cardlist2 = test_board.get_cardlist_vector()[1];

    test_board.reorder_cardlist(cardlist2, cardlist1);

    CHECK(test_board.get_modified());
    std::filesystem::remove("./test-board.xml");
}

TEST_CASE("Checking for board modification: No modification at all") {
    create_dummy_file("TestBoard", "rgba(0,0,0,1)", "test-board.xml");

    Board test_board{"test-board.xml"};
    CHECK_FALSE(test_board.get_modified());
    std::filesystem::remove("./test-board.xml");
}

TEST_CASE("Creating and interactiing with Task objects") {
    Task task{"Write summary of the days do not end"};

    REQUIRE_FALSE(task.get_done());
    task.set_done();
    REQUIRE(task.get_done());
}

TEST_CASE("Adding and removing Task objects from Card objects") {
    Card card{"Hi, my name is Markiplier"};

    auto hi_task = card.add_task(Task{"Hi"});
    auto name_task = card.add_task(Task{"Name"});
    auto mark_task = card.add_task(Task{"Markiplier"});

    REQUIRE(card.get_tasks().size() == 3);

    REQUIRE(card.remove_task(hi_task));
    REQUIRE(card.remove_task(name_task));
    REQUIRE(card.remove_task(mark_task));

    REQUIRE_FALSE(card.remove_task(hi_task));
}

TEST_CASE("Getting the completion of a Card") {
    Card card{"Design some code"};

    Card card2{"Kill your desires"};

    auto task1 = card.add_task(Task{"Choose Language"});
    auto task2 = card.add_task(Task{"Define Performance Requirments"});
    auto task3 = card.add_task(Task{"Write some Tests"});
    auto task4 = card.add_task(Task{"Write the actual code"});

    REQUIRE(card.get_completion() == 0.0);
    REQUIRE(card2.get_completion() == 0.0);

    task1->set_done();
    task2->set_done();
    REQUIRE(card.get_completion() == 50.0);

    task3->set_done();
    task4->set_done();
    REQUIRE(card.get_completion());
}

TEST_CASE("Saving extra task information to cards") {
    std::string xmlString =
        "<board name=\"Test\" background=\"rgb(0,0,0)\">\n"
        "    <list name=\"Cardlist 0\">\n"
        "        <card name=\"Card 0\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "        <card name=\"Card 1\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "        <card name=\"Card 2\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "    </list>\n"
        "    <list name=\"Cardlist 1\">\n"
        "        <card name=\"Card 0\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "        <card name=\"Card 1\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "        <card name=\"Card 2\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "    </list>\n"
        "    <list name=\"Cardlist 2\">\n"
        "        <card name=\"Card 0\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "        <card name=\"Card 1\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "        <card name=\"Card 2\">\n"
        "            <task name=\"Task 0\" done=\"false\"/>\n"
        "            <task name=\"Task 1\" done=\"false\"/>\n"
        "            <task name=\"Task 2\" done=\"false\"/>\n"
        "            <task name=\"Task 3\" done=\"false\"/>\n"
        "            <notes></notes>\n"
        "        </card>\n"
        "    </list>\n"
        "</board>\n";

    write_to("board-progress.xml", xmlString);

    Board board{"board-progress.xml"};
    board.save_as_xml(false);

    REQUIRE(xml_structure(board) == xmlString);
}
