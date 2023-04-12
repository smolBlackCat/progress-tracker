#include <stdexcept>
#include <filesystem>
#include <regex>
#include "board.h"


Board::Board(std::string name, std::string background) :
        Item{name}, background{}, cardlist_vector{} {
    if (!is_background(background)) {
        throw std::domain_error{"given background is not of background type."};
    }

    this->background = background;
}

bool Board::set_background(std::string other) {
    if (is_background(other)) {
        background = other;
        return true;
    }
    return false;
}

std::string Board::get_background() const { 
    return background;
}

std::string Board::xml_structure() const {
    std::string xml_struct;
    tinyxml2::XMLDocument doc;

    tinyxml2::XMLElement* board_element = doc.NewElement("board");
    board_element->SetAttribute("name", name.c_str());
    board_element->SetAttribute("background", background.c_str());
    doc.InsertEndChild(board_element);

    for (auto& cardlist: cardlist_vector) {
        // Create list
        tinyxml2::XMLElement* list_element = doc.NewElement("list");
        list_element->SetAttribute("name", cardlist.get_name().c_str());

        for (auto& card: *cardlist.get_card_vector()) {
            // Create card
            tinyxml2::XMLElement* card_element = doc.NewElement("card");
            card_element->SetAttribute("name", card.get_name().c_str());
            list_element->InsertEndChild(card_element);
        }
        board_element->InsertEndChild(list_element);
    }

    tinyxml2::XMLPrinter printer;

    doc.Print(&printer);

    xml_struct = printer.CStr();
    return xml_struct;
}

bool Board::add_cardlist(CardList& cardlist) {
    try {
        cardlist_vector.push_back(cardlist);
        return true;
    } catch (std::bad_alloc e) {
        return false;
    }
}

bool Board::remove_cardlist(CardList& cardlist) {
    for (size_t i = 0; i < cardlist_vector.size(); i++) {
        if (cardlist == cardlist_vector.at(i)) {
            cardlist_vector.erase(cardlist_vector.begin()+i);
            return true;
        }
    }
    return false;
}

bool Board::is_background(std::string& background) const {
    std::regex rgba_r{"\\(\\d{1,3},\\d{1,3},\\d{1,3},\\d\\)"};

    return std::regex_match(background, rgba_r)
        || std::filesystem::exists(background);
}

Board* board_from_xml(std::string filename) {

    if (!std::filesystem::exists(filename)) return nullptr;

    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename.c_str());

    // Set basic Item attributes: name
    std::string board_name;
    std::string board_background;

    const tinyxml2::XMLElement* board_element = doc.FirstChildElement("board");
    board_name = board_element->FindAttribute("name")->Value();
    board_background = board_element->FindAttribute("background")->Value();

    // Set Board specific attributes: cardlist_vector
    Board* board;
    try {
        board = new Board{board_name, board_background};
    } catch (std::domain_error e) {
        delete board;
        return nullptr;
    }

    const tinyxml2::XMLElement* list_element = board_element->FirstChildElement("list");
    
    while (list_element) {
        CardList cur_cardlist{list_element->Attribute("name")};

        const tinyxml2::XMLElement* card_element = list_element->FirstChildElement("card");
        while(card_element) {
            Card cur_card{card_element->Attribute("name")};
            cur_cardlist.add_card(cur_card);

            card_element = card_element->NextSiblingElement("card");
        }
        board->add_cardlist(cur_cardlist);

        list_element = list_element->NextSiblingElement("list");
    }

    return board;
}