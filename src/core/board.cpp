#include "board.h"

#include <app_info.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <random>
#include <regex>
#include <string>

#include "exceptions.h"

Board::Board() : Item{""} {}

Board::Board(const std::string& name, const std::string& background)
    : Item{name}, background{}, cardlist_vector{}, file_path{} {
    if (!is_background(background)) {
        throw std::invalid_argument{
            "given background is not of background type."};
    }

    this->background = background;
}

Board::Board(const std::string& board_file_path)
    : Item{"none"},
      background{},
      cardlist_vector{},
      file_path{board_file_path} {
    if (!std::filesystem::exists(file_path))
        throw std::invalid_argument{
            std::format("filename {} does not exist.", file_path)};

    tinyxml2::XMLDocument doc;

    auto error_code = doc.LoadFile(file_path.c_str());
    if (error_code != 0) {
        throw std::invalid_argument{std::format(
            "It was not possible to load the file on {}", file_path)};
    }

    // Set basic Item attributes: name

    const tinyxml2::XMLElement* board_element = doc.FirstChildElement("board");

    if (!board_element) {
        throw board_parse_error{
            std::format("{} is not a progress xml file.", file_path)};
    }

    auto board_element_name = board_element->FindAttribute("name");
    auto board_element_background = board_element->FindAttribute("background");

    if (!(board_element_name && board_element_background)) {
        throw board_parse_error{
            std::format("{} is not a progress xml file.", file_path)};
    }

    name = board_element_name->Value();
    background = board_element_background->Value();

    if (name.empty() || !is_background(background)) {
        throw board_parse_error{std::format(
            "{} is not a well formed progress xml file.", file_path)};
    }

    const tinyxml2::XMLElement* list_element =
        board_element->FirstChildElement("list");

    while (list_element) {
        auto cur_cardlist_name = list_element->Attribute("name");
        if (!cur_cardlist_name) {
            throw board_parse_error{
                std::format("{} is not a valid progress xml file.", file_path)};
        }

        CardList cur_cardlist{cur_cardlist_name};

        const tinyxml2::XMLElement* card_element =
            list_element->FirstChildElement("card");
        while (card_element) {
            auto cur_card_name = card_element->Attribute("name");

            if (!cur_card_name) {
                throw board_parse_error{std::format(
                    "{} is not a valid progress xml file.", file_path)};
            }
            cur_cardlist.add_card(Card{cur_card_name});

            card_element = card_element->NextSiblingElement("card");
        }
        cur_cardlist.set_modified(false);
        this->add_cardlist(cur_cardlist);

        list_element = list_element->NextSiblingElement("list");
    }
    modified = false;
}

bool Board::set_background(const std::string& other) {
    bool valid_background = is_background(other);
    if (valid_background) background = other;
    return valid_background;
}

std::string Board::get_background() const { return background; }

bool Board::set_filepath(const std::string& file_path) {
    std::filesystem::path p{file_path};

    if (std::filesystem::exists(file_path) or
        (!std::filesystem::exists(p.parent_path())))
        return false;

    this->file_path = file_path;
    return true;
}

std::string Board::xml_structure() {
    std::string xml_struct;
    tinyxml2::XMLPrinter printer;
    auto doc = xml_doc();
    doc->Print(&printer);
    xml_struct = printer.CStr();
    return xml_struct;
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

bool Board::save_as_xml() {
    auto doc = xml_doc();
    auto result = doc->SaveFile(file_path.c_str());
    return result == tinyxml2::XML_SUCCESS;
}

const std::string Board::get_filepath() const { return file_path; }

std::vector<std::shared_ptr<CardList>>& Board::get_cardlists() {
    return cardlist_vector;
}

bool Board::is_background(const std::string& background) const {
    std::regex rgba_r{"rgba\\(\\d{1,3},\\d{1,3},\\d{1,3},\\d\\)"};
    std::regex rgba1_r{"rgb\\(\\d{1,3},\\d{1,3},\\d{1,3}\\)"};

    return std::regex_match(background, rgba_r) ||
           std::regex_match(background, rgba1_r) ||
           std::filesystem::exists(background);
}

bool Board::cardlists_modified() {
    for (auto& cardlist : cardlist_vector) {
        if (cardlist->is_modified()) {
            return true;
        }
    }
    return false;
}

bool Board::is_modified() { return modified || cardlists_modified(); }

std::string format_basename(std::string basename) {
    std::transform(basename.begin(), basename.end(), basename.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    for (auto& c : basename) {
        if (c == ' ') {
            c = '-';
        }
    }
    return basename;
}

// TODO: Board class deserves a BoardManager class
const std::string Board::new_filename(const std::string& base) {
    std::string boards_dir;
    if (strcmp(FLATPAK, "True") == 0) {
        boards_dir =
            std::string{std::getenv("XDG_CONFIG_HOME")} + "/progress/boards/";
    } else {
        boards_dir =
            std::string{std::getenv("HOME")} + "/.config/progress/boards/";
    }
    std::string filename = "";
    if (std::filesystem::exists(boards_dir)) {
        std::string basename = format_basename(base);
        filename = boards_dir + basename + ".xml";
        if (std::filesystem::exists(filename)) {
            std::random_device r;
            std::default_random_engine e1(r());
            std::uniform_int_distribution<int> uniform_dist(1, 1000);
            int unique_id = uniform_dist(e1);
            filename =
                boards_dir + basename + std::to_string(unique_id) + ".xml";
            while (std::filesystem::exists(filename)) {
                unique_id = uniform_dist(e1);
                filename =
                    boards_dir + basename + std::to_string(unique_id) + ".xml";
            }
        }
    }
    return filename;
}

std::string Board::get_background_type(const std::string& background) {
    std::regex rgba_r{"rgba\\(\\d{1,3},\\d{1,3},\\d{1,3},\\d\\)"};
    std::regex rgba1_r{"rgb\\(\\d{1,3},\\d{1,3},\\d{1,3}\\)"};

    if (std::regex_match(background, rgba_r) ||
        std::regex_match(background, rgba1_r)) {
        return "colour";
    } else if (std::filesystem::exists(background)) {
        return "file";
    }

    return "invalid";
}

std::unique_ptr<tinyxml2::XMLDocument> Board::xml_doc() {
    auto doc = std::make_unique<tinyxml2::XMLDocument>();

    tinyxml2::XMLElement* board_element = doc->NewElement("board");
    board_element->SetAttribute("name", name.c_str());
    board_element->SetAttribute("background", background.c_str());
    doc->InsertEndChild(board_element);

    for (auto& cardlist : cardlist_vector) {
        // Create list
        tinyxml2::XMLElement* list_element = doc->NewElement("list");
        list_element->SetAttribute("name", cardlist->get_name().c_str());

        for (auto& card : cardlist->get_card_vector()) {
            // Create card
            tinyxml2::XMLElement* card_element = doc->NewElement("card");
            card_element->SetAttribute("name", card->get_name().c_str());
            list_element->InsertEndChild(card_element);
        }
        board_element->InsertEndChild(list_element);
    }

    return doc;
}
