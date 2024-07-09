#include "board.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <random>
#include <regex>
#include <string>

#include "exceptions.h"

const std::string Board::BACKGROUND_DEFAULT = "rgba(0,0,0,1)";

Board::Board() : Item{""} {}

Board::Board(const std::string& name, const std::string& background)
    : Item{name} {
    set_background(background);
}

Board::Board(const std::string& board_file_path)
    : Item{""}, file_path{board_file_path} {
    if (!std::filesystem::exists(file_path))
        throw std::invalid_argument{std::format(
            "Progress Board XML file given does not exist: {}", file_path)};

    tinyxml2::XMLDocument doc;

    auto error_code = doc.LoadFile(file_path.c_str());
    if (error_code != 0) {
        throw std::invalid_argument{
            std::format("Failed to load Progress Board XML file given: {}\n"
                        "Error code: {}",
                        file_path, (int)error_code)};
    }

    auto board_element = doc.FirstChildElement("board");

    if (!board_element) {
        throw board_parse_error{
            std::format("Failed to parse given Progress Board XML file: {}\n"
                        "\"board\" element was could not be found",
                        file_path)};
    }

    auto board_element_name = board_element->FindAttribute("name");
    auto board_element_background = board_element->FindAttribute("background");

    if (!(board_element_name && board_element_background)) {
        std::string missing_attr = board_element_name ? "background" : "name";
        throw board_parse_error{
            std::format("Failed to parse given Progress Board XML file: {}\n"
                        "\"{}\" attribute could not be found",
                        missing_attr, file_path)};
    }

    name = board_element_name->Value();

    // TODO: It'd be good practise to log a warning message informing it was
    // not possible to set a background, and the default one is going to be set
    set_background(board_element_background->Value());

    if (name.empty()) {
        throw board_parse_error{
            std::format("Failed to parse given Progress Board XML file: {}\n"
                        "Boards with empty names are not allowed",
                        file_path)};
    }

    auto list_element = board_element->FirstChildElement("list");

    while (list_element) {
        auto cur_cardlist_name = list_element->Attribute("name");
        auto cur_cardlist_color = list_element->Attribute("color");

        if (!cur_cardlist_name) {
            throw board_parse_error{std::format(
                "Failed to parse given Progress Board XML file: {}\n"
                "A \"list\" element on line {} failed to parsed.",
                file_path, list_element->GetLineNum())};
        }

        // The extra checking ensures retro compatibiity with Progress 1.0 Board
        // files
        CardList cur_cardlist{
            cur_cardlist_name,
            cur_cardlist_color ? Gdk::RGBA{cur_cardlist_color} : NO_COLOR};
        auto card_element = list_element->FirstChildElement("card");

        while (card_element) {
            auto cur_card_name = card_element->Attribute("name");
            auto cur_card_color = card_element->Attribute("color");

            if (!cur_card_name) {
                throw board_parse_error{std::format(
                    "Failed to parse given Progress Board XML file: {}\n"
                    "Failed to load {} \"list\" element\n"
                    "\"card\" element on line {} has no name attribute",
                    file_path, cur_cardlist_name, card_element->GetLineNum())};
            }

            cur_cardlist.add_card(
                Card{cur_card_name,
                     cur_card_color ? Gdk::RGBA{cur_card_color} : NO_COLOR});
            card_element = card_element->NextSiblingElement("card");
        }
        cur_cardlist.set_modified(false);
        add_cardlist(cur_cardlist);

        list_element = list_element->NextSiblingElement("list");
    }
    modified = false;
}

BackgroundType Board::set_background(const std::string& other, bool modify) {
    BackgroundType bg_type = Board::get_background_type(other);

    switch (bg_type) {
        case BackgroundType::IMAGE:
        case BackgroundType::COLOR: {
            background = other;
        } break;
        default: {
            background = Board::BACKGROUND_DEFAULT;
        } break;
    }

    modified = modify ? true : modified;

    return bg_type;
}

std::string Board::get_background() const { return background; }

bool Board::set_filepath(const std::string& file_path, bool create_dirs) {
    std::filesystem::path p{file_path};

    if (std::filesystem::exists(p.parent_path())) {
        if (!std::filesystem::exists(p)) {
            this->file_path = file_path;
            return true;
        }
    } else {
        if (create_dirs) {
            std::filesystem::create_directories(p.parent_path());
            this->file_path = file_path;
            return true;
        }
    }

    return false;
}

std::shared_ptr<CardList> Board::add_cardlist(const CardList& cardlist) {
    std::shared_ptr<CardList> new_cardlist =
        std::make_shared<CardList>(cardlist);
    if (new_cardlist) {
        cardlist_vector.push_back(new_cardlist);
        modified = true;
    }
    return new_cardlist;
}

bool Board::remove_cardlist(const CardList& cardlist) {
    for (size_t i = 0; i < cardlist_vector.size(); i++) {
        if (cardlist == (*cardlist_vector.at(i))) {
            cardlist_vector.erase(cardlist_vector.begin() + i);
            modified = true;
            return true;
        }
    }
    return false;
}

void Board::reorder_cardlist(std::shared_ptr<CardList> next,
                             std::shared_ptr<CardList> sibling) {
    size_t next_i = -1;
    size_t sibling_i = -1;

    for (size_t i = 0; i < cardlist_vector.size(); i++) {
        if (*cardlist_vector[i] == *next) {
            next_i = i;
        }
        if (*cardlist_vector[i] == *sibling) {
            sibling_i = i;
        }
    }

    if (next_i == -1 || sibling_i == -1) {
        throw std::invalid_argument{
            "Either next or sibling are not children of this cardlist"};
    }

    auto next_it = std::next(cardlist_vector.begin(), next_i);
    std::shared_ptr<CardList> temp_v = cardlist_vector[next_i];
    cardlist_vector.erase(next_it);

    // Support for right to left drags and drops
    if (next_i < sibling_i) {
        sibling_i -= 1;
    }

    if (sibling_i == cardlist_vector.size() - 1) {
        cardlist_vector.push_back(temp_v);
    } else {
        auto sibling_it = std::next(cardlist_vector.begin(), sibling_i + 1);
        cardlist_vector.insert(sibling_it, temp_v);
    }
    modified = true;
}

bool Board::save_as_xml(bool create_dirs) {
    auto doc = std::make_unique<tinyxml2::XMLDocument>();

    tinyxml2::XMLElement* board_element = doc->NewElement("board");
    board_element->SetAttribute("name", name.c_str());
    board_element->SetAttribute("background", background.c_str());
    doc->InsertEndChild(board_element);

    for (auto& cardlist : cardlist_vector) {
        tinyxml2::XMLElement* list_element = doc->NewElement("list");
        list_element->SetAttribute("name", cardlist->get_name().c_str());
        list_element->SetAttribute("color",
                                   cardlist->get_color().to_string().c_str());
        cardlist->set_modified(false);

        for (auto& card : cardlist->get_card_vector()) {
            tinyxml2::XMLElement* card_element = doc->NewElement("card");
            card_element->SetAttribute("name", card->get_name().c_str());
            card_element->SetAttribute("color",
                                       card->get_color().to_string().c_str());
            list_element->InsertEndChild(card_element);
            card->set_modified(false);
        }
        board_element->InsertEndChild(list_element);
    }
    set_modified(false);

    std::filesystem::path p{file_path};
    if (!std::filesystem::exists(p.parent_path())) {
        if (create_dirs)
            std::filesystem::create_directories(p.parent_path());
        else
            return false;
    }

    return doc->SaveFile(file_path.c_str()) == tinyxml2::XML_SUCCESS;
}

std::string Board::get_filepath() const { return file_path; }

const std::vector<std::shared_ptr<CardList>>& Board::get_cardlist_vector() {
    return cardlist_vector;
}

bool Board::get_modified() {
    for (auto& cardlist : cardlist_vector) {
        if (cardlist->get_modified()) {
            return true;
        }
    }
    return modified;
}

BackgroundType Board::get_background_type(const std::string& background) {
    std::regex rgba_r{"rgba\\(\\d{1,3},\\d{1,3},\\d{1,3},\\d\\)"};
    std::regex rgba1_r{"rgb\\(\\d{1,3},\\d{1,3},\\d{1,3}\\)"};
    if (std::regex_match(background, rgba_r) ||
        std::regex_match(background, rgba1_r)) {
        return BackgroundType::COLOR;
    } else if (std::filesystem::exists(background)) {
        return BackgroundType::IMAGE;
    }
    return BackgroundType::INVALID;
}
