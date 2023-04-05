#include <stdexcept>
#include <filesystem>
#include <regex>
#include "board.h"


Board::Board(std::string name, std::string background) :
        Item{name}, background{}, cardlist_vector{} {
    if (!is_background(background)) {
        throw std::domain_error{"given background is not of background type."};
    }
}

Board::Board(std::string name, std::string background, unsigned long long id) :
        Item{name, id}, background{background}, cardlist_vector{} {
    if (!is_background(background)) {
        throw std::domain_error{"given background is not of background type."};
    }
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

std::string Board::fetch_xml() const {
    std::string xml_code = "<xml version=\"1.0\">\n"
        "<board name=\"" + name + "\" background=\"" + background + "\">\n";
    
    for (size_t i = 0; i < cardlist_vector.size(); i++) {
        xml_code += cardlist_vector.at(i).fetch_xml() + "\n";
    }
    xml_code += "</board>";
    
    return xml_code;
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