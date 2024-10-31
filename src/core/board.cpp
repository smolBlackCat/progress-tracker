#include "board.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <regex>
#include <set>

#include "exceptions.h"

BoardBackend::BoardBackend(BackendType backend_type,
                           const std::map<std::string, std::string>& settings)
    : type{backend_type} {
    switch (backend_type) {
        case BackendType::LOCAL: {
            this->settings["filepath"] =
                settings.contains("filepath") ? settings.at("filepath") : "";
            break;
        }
        case BackendType::CALDAV:
        case BackendType::NEXTCLOUD:
            throw std::invalid_argument{"Not implemented"};
    }
}

Board BoardBackend::load() {
    switch (type) {
        case BackendType::LOCAL: {
            if (!std::filesystem::exists(settings["filepath"]))
                throw std::invalid_argument{std::format(
                    "Progress Board XML file given does not exist: {}",
                    settings["filepath"])};

            tinyxml2::XMLDocument doc;

            auto error_code = doc.LoadFile(settings["filepath"].c_str());
            if (error_code != 0) {
                throw std::invalid_argument{std::format(
                    "Failed to load Progress Board XML file given: {}\n"
                    "Error code: {}",
                    settings["filepath"], (int)error_code)};
            }

            auto board_element = doc.FirstChildElement("board");

            if (!board_element) {
                throw board_parse_error{std::format(
                    "Failed to parse given Progress Board XML file: {}\n"
                    "\"board\" element was could not be found",
                    settings["filepath"])};
            }

            auto board_element_name = board_element->FindAttribute("name");
            auto board_element_background =
                board_element->FindAttribute("background");

            if (!(board_element_name && board_element_background)) {
                std::string missing_attr =
                    board_element_name ? "background" : "name";
                throw board_parse_error{std::format(
                    "Failed to parse given Progress Board XML file: {}\n"
                    "\"{}\" attribute could not be found",
                    missing_attr, settings["filepath"])};
            }

            std::string name = board_element_name->Value();
            std::string background = board_element_background->Value();

            if (name.empty()) {
                throw board_parse_error{std::format(
                    "Failed to parse given Progress Board XML file: {}\n"
                    "Boards with empty names are not allowed",
                    settings["filepath"])};
            }

            Board board{*this};
            board.set_name(name);
            board.set_background(background);
            auto lm_filepath =
                std::chrono::clock_cast<std::chrono::system_clock,
                                        std::chrono::file_clock>(
                    std::filesystem::last_write_time(settings["filepath"]));
            board.last_modified =
                std::chrono::floor<std::chrono::seconds>(lm_filepath);

            board.modified = false;
            return board;
        }
        case BackendType::CALDAV:
        case BackendType::NEXTCLOUD:
            throw std::invalid_argument{"Not implemented"};
    }
}

Board BoardBackend::create(const std::string& name,
                           const std::string& background) {
    Board board{*this};

    // Because a new Board is created from scratch, it is essentially loaded
    board.fully_loaded = true;

    board.set_name(name);
    board.set_background(background);
    board.set_modified(false);

    return board;
}

std::string BoardBackend::get_attribute(const std::string& key) const {
    if (settings.contains(key)) {
        return settings.at(key);
    } else {
        return "";
    }
}

bool BoardBackend::set_attribute(const std::string& key,
                                 const std::string& value) {
    switch (type) {
        case BackendType::LOCAL: {
            const std::set<std::string> local_attributes = {"filepath"};
            if (local_attributes.contains(key) && !value.empty()) {
                settings[key] = value;
            }
            return true;
        }
        case BackendType::CALDAV:
        case BackendType::NEXTCLOUD:
            return false;
    }
}

bool BoardBackend::save(Board& board) {
    if (board.fully_loaded) {
        switch (type) {
            case BackendType::LOCAL:
                return save_xml(board);
            case BackendType::CALDAV:
                return save_caldav(board);
            case BackendType::NEXTCLOUD:
                return save_nextcloud(board);
        }
    }
    return false;
}

void BoardBackend::load_cardlists(Board& board) {
    if (!board.fully_loaded) {
        tinyxml2::XMLDocument doc;
        doc.LoadFile(settings["filepath"].c_str());
        board.fully_loaded = true;
        auto list_element =
            doc.FirstChildElement("board")->FirstChildElement("list");

        while (list_element) {
            auto cur_cardlist_name = list_element->Attribute("name");

            if (!cur_cardlist_name) {
                throw board_parse_error{std::format(
                    "Failed to parse given Progress Board XML file: {}\n"
                    "A \"list\" element on line {} failed to parsed.",
                    settings["filepath"], list_element->GetLineNum())};
            }

            CardList cur_cardlist{cur_cardlist_name};
            auto card_element = list_element->FirstChildElement("card");

            while (card_element) {
                auto cur_card_name = card_element->Attribute("name");
                auto cur_card_color = card_element->Attribute("color");
                auto cur_card_due_date = card_element->Attribute("due");
                auto cur_card_complete =
                    card_element->BoolAttribute("complete");

                if (!cur_card_name) {
                    throw board_parse_error{std::format(
                        "Failed to parse given Progress Board XML file: "
                        "{}\n"
                        "Failed to load {} \"list\" element\n"
                        "\"card\" element on line {} has no name attribute",
                        settings["filepath"], cur_cardlist_name,
                        card_element->GetLineNum())};
                }

                Date date{};

                if (cur_card_due_date) {
                    const std::regex date_r{"\\d\\d\\d\\d-\\d\\d-\\d\\d"};

                    if (!std::regex_match(cur_card_due_date, date_r)) {
                        throw board_parse_error{std::format(
                            "Invalid due date at Card {}", cur_card_name)};
                    }
                    std::chrono::sys_seconds secs;

                    std::istringstream{std::string{cur_card_due_date}} >>
                        std::chrono::parse("%F", secs);
                    Date date{std::chrono::floor<std::chrono::days>(secs)};
                    date = date;
                }

                Card cur_card{
                    cur_card_name,
                    date,
                    cur_card_complete,
                    cur_card_color ? string_to_color(cur_card_color) : NO_COLOR,
                };

                auto task_element = card_element->FirstChildElement("task");
                while (task_element) {
                    cur_card.add(Task{task_element->Attribute("name"),
                                      task_element->BoolAttribute("done")});
                    task_element = task_element->NextSiblingElement("task");
                }

                auto notes_element = card_element->FirstChildElement("notes");
                if (notes_element) {
                    cur_card.set_notes(!notes_element->GetText()
                                           ? ""
                                           : notes_element->GetText());
                }

                cur_card.set_modified(false);

                cur_cardlist.add(cur_card);
                card_element = card_element->NextSiblingElement("card");
            }
            cur_cardlist.set_modified(false);
            board.add(cur_cardlist);

            list_element = list_element->NextSiblingElement("list");
        }
        board.set_modified(false);
    }
}

BackendType BoardBackend::get_type() const noexcept { return type; }

bool BoardBackend::save_xml(Board& board) {
    auto doc = std::make_unique<tinyxml2::XMLDocument>();

    tinyxml2::XMLElement* board_element = doc->NewElement("board");
    board_element->SetAttribute("name", board.get_name().c_str());
    board_element->SetAttribute("background", board.get_background().c_str());
    doc->InsertEndChild(board_element);

    for (auto& cardlist : board.get_cardlists()) {
        tinyxml2::XMLElement* list_element = doc->NewElement("list");
        list_element->SetAttribute("name", cardlist->get_name().c_str());
        cardlist->set_modified(false);

        for (auto& card : cardlist->get_cards()) {
            tinyxml2::XMLElement* card_element = doc->NewElement("card");
            card_element->SetAttribute("name", card->get_name().c_str());
            if (card->is_color_set())
                card_element->SetAttribute(
                    "color", color_to_string(card->get_color()).c_str());
            auto date = card->get_due_date();
            if (date.ok()) {
                card_element->SetAttribute("due",
                                           std::format("{}", date).c_str());
                card_element->SetAttribute("complete", card->get_complete());
            }

            // Add tasks
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
            card->set_modified(false);
        }
        board_element->InsertEndChild(list_element);
    }
    board.set_modified(false);

    std::filesystem::path p{settings["filepath"]};
    if (p.has_parent_path() && !std::filesystem::exists(p.parent_path())) {
        std::filesystem::create_directories(p.parent_path());
    }

    return doc->SaveFile(settings["filepath"].c_str()) == tinyxml2::XML_SUCCESS;
}

bool BoardBackend::save_caldav(Board& board) { return false; }

bool BoardBackend::save_nextcloud(Board& board) { return false; }

const std::string Board::BACKGROUND_DEFAULT = "rgba(0,0,0,1)";

Board::Board(BoardBackend& backend) : Item{""}, backend{backend} {}

Board::~Board() { std::cout << name << " has been destroyed" << '\n'; }

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

    modified = modify ? modify : modified;

    return bg_type;
}

const std::string& Board::get_background() const { return background; }

std::shared_ptr<CardList> Board::add(const CardList& cardlist) {
    if (fully_loaded) {
        for (auto& ccardlist : cardlists) {
            if (*ccardlist == cardlist) {
                return nullptr;
            }
        }

        try {
            std::shared_ptr<CardList> new_cardlist =
                std::make_shared<CardList>(cardlist);
            cardlists.push_back(new_cardlist);
            modified = true;
            return new_cardlist;
        } catch (std::bad_alloc& err) {
            return nullptr;
        }
    }
    return nullptr;
}

bool Board::remove(const CardList& cardlist) {
    if (fully_loaded) {
        for (size_t i = 0; i < cardlists.size(); i++) {
            if (cardlist == (*cardlists.at(i))) {
                cardlists.erase(cardlists.begin() + i);
                modified = true;
                return true;
            }
        }
    }
    return false;
}

void Board::reorder(const CardList& next, const CardList& sibling) {
    if (fully_loaded) {
        ssize_t next_i = -1;
        ssize_t sibling_i = -1;

        ssize_t c = 0;
        for (auto& cardlist : cardlists) {
            if (*cardlist == next) {
                next_i = c;
            } else if (*cardlist == sibling) {
                sibling_i = c;
            }
            c++;
        }

        bool any_absent_item = next_i + sibling_i < 0;
        bool is_same_item = next_i == sibling_i;
        bool already_in_order = next_i - sibling_i == 1;
        if (any_absent_item || is_same_item || already_in_order) {
            return;
        }

        auto next_it = std::next(cardlists.begin(), next_i);
        std::shared_ptr<CardList> temp_v = cardlists[next_i];
        cardlists.erase(next_it);

        // Support for right to left drags and drops
        if (next_i < sibling_i) {
            sibling_i -= 1;
        }

        if (sibling_i == cardlists.size() - 1) {
            cardlists.push_back(temp_v);
        } else {
            auto sibling_it = std::next(cardlists.begin(), sibling_i + 1);
            cardlists.insert(sibling_it, temp_v);
        }
        modified = true;
    }
}

bool Board::save() { return backend.save(*this); }

void Board::load() { backend.load_cardlists(*this); }

const std::vector<std::shared_ptr<CardList>>& Board::get_cardlists() {
    return cardlists;
}

void Board::set_modified(bool modified) {
    if (fully_loaded) {
        Item::set_modified(modified);

        for (auto& cardlist : cardlists) {
            cardlist->set_modified(modified);
        }
    }
}

time_point<system_clock, seconds> Board::get_last_modified() const {
    return last_modified;
}

bool Board::is_loaded() { return fully_loaded; }

bool Board::get_modified() {
    for (auto& cardlist : cardlists) {
        if (cardlist->get_modified()) {
            modified = true;
            break;
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
