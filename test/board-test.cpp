#include "cardlist.h"
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

#include "board.h"

TEST_CASE("Board default constructor", "[Board]") {
    Board board;
    REQUIRE(board.get_name() == "");
    REQUIRE(board.get_background() == Board::BACKGROUND_DEFAULT);
    REQUIRE(board.get_filepath() == "");
    REQUIRE(board.get_cardlist_vector().empty());
    REQUIRE(!board.get_modified());
}

TEST_CASE("Board parameterized constructor", "[Board]") {
    std::string name = "Test Board";
    std::string background = "rgba(255,0,0,1)";
    Board board(name, background);
    REQUIRE(board.get_name() == name);
    REQUIRE(board.get_background() == background);
    REQUIRE(board.get_cardlist_vector().empty());
    REQUIRE_FALSE(board.get_modified());
}

TEST_CASE("Board XML constructor", "[Board]") {
    std::string xml_path = "test_board.xml";
    std::string xml_content =
        "<board name=\"Test Board\" background=\"rgba(255,0,0,1)\">"
        "<list name=\"To Do\">"
        "<card name=\"Task 1\" color=\"rgba(0,0,255,1)\">"
        "<task name=\"Subtask 1\" done=\"false\" />"
        "<notes>Some notes</notes>"
        "</card>"
        "</list>"
        "</board>";

    // Write XML content to a file
    std::ofstream file(xml_path);
    file << xml_content;
    file.close();

    Board board(xml_path);
    REQUIRE(board.get_name() == "Test Board");
    REQUIRE(board.get_background() == "rgba(255,0,0,1)");
    REQUIRE(board.get_cardlist_vector().size() == 1);
    REQUIRE(board.get_cardlist_vector()[0]->get_name() == "To Do");

    // Clean up
    std::filesystem::remove(xml_path);
}

TEST_CASE("Board set and get background", "[Board]") {
    Board board;
    REQUIRE(board.get_background() == Board::BACKGROUND_DEFAULT);

    board.set_background("rgba(255,255,255,1)");
    REQUIRE(board.get_background() == "rgba(255,255,255,1)");
    REQUIRE(board.get_modified());

    board.set_background("invalid_background");
    REQUIRE(board.get_background() == Board::BACKGROUND_DEFAULT);
    REQUIRE(board.get_modified());
}

TEST_CASE("Board set and get filepath", "[Board]") {
    Board board;
    std::string file_path = "./test_board.xml";
    REQUIRE(board.set_filepath(file_path));
    REQUIRE(board.get_filepath() == file_path);

    REQUIRE(board.set_filepath("invalid_path/test_board.xml", false) == false);
    REQUIRE(board.get_filepath() == file_path);
}

TEST_CASE("Board add and remove cardlist", "[Board]") {
    Board board;
    CardList cardlist("To Do");

    auto added_cardlist = board.add_cardlist(cardlist);
    REQUIRE(added_cardlist->get_name() == "To Do");
    REQUIRE(board.get_cardlist_vector().size() == 1);
    REQUIRE(board.get_modified());

    REQUIRE(board.remove_cardlist(cardlist) == true);
    REQUIRE(board.get_cardlist_vector().empty());
    REQUIRE(board.get_modified());
}

TEST_CASE("Board save as XML", "[Board]") {
    Board board("Test Board", "rgba(255,0,0,1)");
    std::string file_path = "test_save_board.xml";
    board.set_filepath(file_path);

    CardList cardlist("To Do");
    Card card("Task 1", Color{0, 0, 255, 1});
    cardlist.add_card(card);
    board.add_cardlist(cardlist);

    REQUIRE(board.save_as_xml());
    REQUIRE(std::filesystem::exists(file_path));

    // Load the saved file and verify its contents
    tinyxml2::XMLDocument doc;
    REQUIRE(doc.LoadFile(file_path.c_str()) == tinyxml2::XML_SUCCESS);

    auto board_element = doc.FirstChildElement("board");
    REQUIRE(board_element != nullptr);
    REQUIRE(board_element->Attribute("name") == std::string("Test Board"));
    REQUIRE(board_element->Attribute("background") ==
            std::string("rgba(255,0,0,1)"));

    auto list_element = board_element->FirstChildElement("list");
    REQUIRE(list_element != nullptr);
    REQUIRE(list_element->Attribute("name") == std::string("To Do"));

    auto card_element = list_element->FirstChildElement("card");
    REQUIRE(card_element != nullptr);
    REQUIRE(card_element->Attribute("name") == std::string("Task 1"));
    auto color = card_element->Attribute("color");
    REQUIRE(color == std::string("rgb(0,0,255)"));

    // Clean up
    std::filesystem::remove(file_path);
}

TEST_CASE("Board save as XML (2)", "[Board]") {
    Board board("Test Board", "rgba(255,0,0,1)");
    std::string file_path =
        (std::filesystem::current_path() / "test_save_board.xml").string();
    board.set_filepath(file_path);

    CardList cardlist("To Do");
    Card card("Task 1", Color{0, 0, 255, 1});
    cardlist.add_card(card);
    board.add_cardlist(cardlist);

    REQUIRE(board.save_as_xml());
    REQUIRE(std::filesystem::exists(file_path));

    // Load the saved file and verify its contents
    tinyxml2::XMLDocument doc;
    REQUIRE(doc.LoadFile(file_path.c_str()) == tinyxml2::XML_SUCCESS);

    auto board_element = doc.FirstChildElement("board");
    REQUIRE(board_element != nullptr);
    REQUIRE(board_element->Attribute("name") == std::string("Test Board"));
    REQUIRE(board_element->Attribute("background") ==
            std::string("rgba(255,0,0,1)"));

    auto list_element = board_element->FirstChildElement("list");
    REQUIRE(list_element != nullptr);
    REQUIRE(list_element->Attribute("name") == std::string("To Do"));

    auto card_element = list_element->FirstChildElement("card");
    REQUIRE(card_element != nullptr);
    REQUIRE(card_element->Attribute("name") == std::string("Task 1"));
    REQUIRE(card_element->Attribute("color") == std::string("rgb(0,0,255)"));

    // Clean up
    std::filesystem::remove(file_path);
}

TEST_CASE("Board save as XML (3)", "[Board]") {
    Board board("Test Board", "rgba(255,0,0,1)");
    std::string file_path = "folder/test_board.xml";
    board.set_filepath(file_path);

    CardList cardlist("To Do");
    Card card("Task 1", Color{0, 0, 255, 1});
    cardlist.add_card(card);
    board.add_cardlist(cardlist);

    REQUIRE(board.save_as_xml());
    REQUIRE(std::filesystem::exists(file_path));

    // Load the saved file and verify its contents
    tinyxml2::XMLDocument doc;
    REQUIRE(doc.LoadFile(file_path.c_str()) == tinyxml2::XML_SUCCESS);

    auto board_element = doc.FirstChildElement("board");
    REQUIRE(board_element != nullptr);
    REQUIRE(board_element->Attribute("name") == std::string("Test Board"));
    REQUIRE(board_element->Attribute("background") ==
            std::string("rgba(255,0,0,1)"));

    auto list_element = board_element->FirstChildElement("list");
    REQUIRE(list_element != nullptr);
    REQUIRE(list_element->Attribute("name") == std::string("To Do"));

    auto card_element = list_element->FirstChildElement("card");
    REQUIRE(card_element != nullptr);
    REQUIRE(card_element->Attribute("name") == std::string("Task 1"));
    REQUIRE(card_element->Attribute("color") == std::string("rgb(0,0,255)"));

    // Clean up
    std::filesystem::remove(file_path);
}

TEST_CASE("Board get background type", "[Board]") {
    REQUIRE(Board::get_background_type("rgba(255,0,0,1)") ==
            BackgroundType::COLOR);
    REQUIRE(Board::get_background_type("rgb(255,0,0)") ==
            BackgroundType::COLOR);
    REQUIRE(Board::get_background_type("invalid_background") ==
            BackgroundType::INVALID);

    std::string img_path = "test_image.png";
    std::ofstream img_file{img_path};
    img_file.close();
    REQUIRE(Board::get_background_type(img_path) == BackgroundType::IMAGE);
    std::filesystem::remove(img_path);
}

TEST_CASE("Reordering Cardlists within a Board", "[Board]") {
    Board board{"Board", "rgb(1,1,1)"};

    CardList cardlist1{"TODO"};
    CardList cardlist2{"DOING"};
    CardList cardlist3{"DONE"};

    board.add_cardlist(cardlist1);
    board.add_cardlist(cardlist2);
    board.add_cardlist(cardlist3);

    REQUIRE(*board.get_cardlist_vector().at(0) == cardlist1);
    REQUIRE(*board.get_cardlist_vector().at(1) == cardlist2);
    REQUIRE(*board.get_cardlist_vector().at(2) == cardlist3);

    board.reorder_cardlist(cardlist1, cardlist3);

    REQUIRE(*board.get_cardlist_vector().at(2) == cardlist1);
    REQUIRE(*board.get_cardlist_vector().at(0) == cardlist2);
    REQUIRE(*board.get_cardlist_vector().at(1) == cardlist3);
}