#include <app_info.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <regex>
#include <thread>

#include "board-manager.h"

namespace fs = std::filesystem;

#ifdef DEVELOPMENT
constexpr const char* BOARDS_FOLDER = "/progress-debug/boards/";
constexpr const char* BOARDS_FOLDER_WIN32 = "\\Progress Debug\\Boards\\";
#else
constexpr const char* BOARDS_FOLDER = "/progress/boards/";
constexpr const char* BOARDS_FOLDER_WIN32 = "\\Progress\\Boards\\";
#endif

std::string progress_boards_folder_old() {
#ifdef FLATPAK
    return std::string{std::getenv("XDG_CONFIG_HOME")} + BOARDS_FOLDER;
#elif WIN32
    return std::string{std::getenv("APPDATA")} + BOARDS_FOLDER_WIN32;
#else
    return std::string{std::getenv("HOME")} +
           std::format("/.config{}", BOARDS_FOLDER);
#endif
}

std::string progress_boards_folder() {
#ifdef FLATPAK
    return std::string{std::getenv("XDG_DATA_HOME")} + BOARDS_FOLDER;
#elif WIN32
    return std::string{std::getenv("APPDATA")} + BOARDS_FOLDER_WIN32;
#else
    return std::string{std::getenv("HOME")} +
           std::format("/.local/share{}", BOARDS_FOLDER);
#endif
}

std::string gen_filename(const std::string& boards_dir, const Board& board) {
    std::string filename = "";

    if (fs::exists(boards_dir)) {
        filename = boards_dir + board.get_id().str() + ".xml";
    }

    return filename;
}

std::shared_ptr<Board> unitialized_board(const std::string& filename) {
    spdlog::get("core")->debug(
        "[BoardManager] BoardManager is loading board from file: {}", filename);
    if (!fs::exists(filename))
        throw std::invalid_argument{std::format(
            "Progress Board XML file given does not exist: {}", filename)};

    tinyxml2::XMLDocument doc;

    auto error_code = doc.LoadFile(filename.c_str());
    if (error_code != 0) {
        throw std::invalid_argument{
            std::format("Failed to load Progress Board XML file given: {}\n"
                        "Error code: {}",
                        filename, (int)error_code)};
    }

    auto board_element = doc.FirstChildElement("board");

    if (!board_element) {
        throw std::invalid_argument{
            std::format("Failed to parse given Progress Board XML file: {}\n"
                        "\"board\" element was could not be found",
                        filename)};
    }

    auto board_element_name = board_element->FindAttribute("name");
    auto board_element_background = board_element->FindAttribute("background");
    auto board_element_uuid = board_element->FindAttribute("uuid");

    if (!(board_element_name && board_element_background)) {
        std::string missing_attr = board_element_name ? "background" : "name";
        throw std::invalid_argument{
            std::format("Failed to parse given Progress Board XML file: {}\n"
                        "\"{}\" attribute could not be found",
                        missing_attr, filename)};
    }

    std::string name = board_element_name->Value();
    std::string background = board_element_background->Value();

    // There might exist some boards that do not keep track of uuids
    xg::Guid uuid = board_element_uuid ? xg::Guid(board_element_uuid->Value())
                                       : xg::newGuid();

    if (name.empty()) {
        throw std::invalid_argument{
            std::format("Failed to parse given Progress Board XML file: {}\n"
                        "Boards with empty names are not allowed",
                        filename)};
    }

    std::shared_ptr<Board> board =
        std::make_shared<Board>(name, background, uuid);
    auto lm_filepath = std::chrono::clock_cast<std::chrono::system_clock,
                                               std::chrono::file_clock>(
        std::filesystem::last_write_time(filename));
    board->m_last_modified =
        std::chrono::floor<std::chrono::seconds>(lm_filepath);

    return board;
}

void full_load(const std::string& filename,
               const std::shared_ptr<Board>& board) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError status = doc.LoadFile(filename.c_str());
    if (status != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error{
            std::format("Failed to parse given Progress Board XML file: {}\n"
                        "XML parsing error: {}",
                        filename, static_cast<int>(status))};
    }

    auto list_element =
        doc.FirstChildElement("board")->FirstChildElement("list");

    while (list_element) {
        auto cur_cardlist_name = list_element->Attribute("name");
        auto cur_cardlist_uuid = list_element->Attribute("uuid");

        if (!cur_cardlist_name) {
            throw std::invalid_argument{std::format(
                "Failed to parse given Progress Board XML file: {}\n"
                "A \"list\" element on line {} failed to parsed.",
                filename, list_element->GetLineNum())};
        }

        CardList cur_cardlist{
            cur_cardlist_name,
            cur_cardlist_uuid ? xg::Guid{cur_cardlist_uuid} : xg::newGuid()};
        auto card_element = list_element->FirstChildElement("card");

        while (card_element) {
            auto cur_card_name = card_element->Attribute("name");
            auto cur_card_color = card_element->Attribute("color");
            auto cur_card_due_date = card_element->Attribute("due");
            auto cur_card_complete = card_element->BoolAttribute("complete");
            auto cur_card_uuid = card_element->Attribute("uuid");

            if (!cur_card_name) {
                throw std::invalid_argument{std::format(
                    "Failed to parse given Progress Board XML file: "
                    "{}\n"
                    "Failed to load {} \"list\" element\n"
                    "\"card\" element on line {} has no name attribute",
                    filename, cur_cardlist_name, card_element->GetLineNum())};
            }

            Date date{};

            if (cur_card_due_date) {
                const std::regex date_r{"\\d\\d\\d\\d-\\d\\d-\\d\\d"};

                if (!std::regex_match(cur_card_due_date, date_r)) {
                    throw std::invalid_argument{std::format(
                        "Invalid due date at Card {}", cur_card_name)};
                }
                std::chrono::sys_seconds secs;

                std::istringstream{std::string{cur_card_due_date}} >>
                    std::chrono::parse("%F", secs);
                date = Date{std::chrono::floor<std::chrono::days>(secs)};
            }

            Card cur_card{
                cur_card_name,
                date,
                cur_card_uuid ? xg::Guid{cur_card_uuid} : xg::newGuid(),
                cur_card_complete,
                cur_card_color ? string_to_color(cur_card_color) : NO_COLOR,
            };

            auto task_element = card_element->FirstChildElement("task");
            while (task_element) {
                auto task_element_uuid = task_element->Attribute("uuid");
                cur_card.container().append(
                    Task{task_element->Attribute("name"),
                         task_element_uuid ? xg::Guid{task_element_uuid}
                                           : xg::newGuid(),
                         task_element->BoolAttribute("done")});
                task_element = task_element->NextSiblingElement("task");
            }

            auto notes_element = card_element->FirstChildElement("notes");
            if (notes_element) {
                cur_card.set_notes(
                    !notes_element->GetText() ? "" : notes_element->GetText());
            }

            cur_card.modify(false);
            cur_card.container().modify(false);

            cur_cardlist.container().append(cur_card);
            card_element = card_element->NextSiblingElement("card");
        }
        cur_cardlist.modify(false);
        cur_cardlist.container().modify(false);

        board->container().append(cur_cardlist);

        list_element = list_element->NextSiblingElement("list");
    }

    board->modify(false);
    board->container().modify(false);

    spdlog::get("core")->debug(
        "BoardManager has loaded all cardlists for board \"{}\"",
        board->get_name());
}

BoardManager::BoardManager() : BoardManager{progress_boards_folder()} {}

BoardManager::BoardManager(const std::string& board_dir)
    : BOARD_DIR{board_dir} {
    if (!(fs::exists(BOARD_DIR) || fs::create_directories(BOARD_DIR))) {
        throw std::runtime_error{
            "Failed to load boards: Boards folder cannot be resolved"};
    }

    spdlog::get("core")->info(
        "[BoardManager] Loading boards from local storage: {}", board_dir);
    std::thread([this]() {
        std::lock_guard<std::mutex> lock(this->valid_mutex);
        for (const auto& dir_entry :
             std::filesystem::directory_iterator(BOARD_DIR)) {
            const std::string board_filename = dir_entry.path().string();
            if (board_filename.ends_with(".xml")) {
                try {
                    m_local_boards.push_back(
                        LocalBoard{board_filename,
                                   unitialized_board(board_filename), false});
                } catch (std::invalid_argument& err) {
                    spdlog::get("core")->error("Failed to load board: {}",
                                               err.what());
                }
            }
        }
        m_loaded = true;
    }).detach();

    spdlog::get("core")->info(
        "[BoardManager] Local boards have been successfully loaded", BOARD_DIR);
}

std::shared_ptr<Board> BoardManager::local_open(const std::string& filename) {
    for (auto it = m_local_boards.begin(); it != m_local_boards.end(); it++) {
        if (it->filename == filename) {
            try {
                if (!it->is_open) {
                    full_load(filename, it->board);
                    it->is_open = true;
                }
                return it->board;
            } catch (std::invalid_argument& err) {
                // TODO: Signaling may be good to show a dialog where the error
                // was since it does not mean that the file is corrupted, it
                // just means the file is not well-formed

                spdlog::get("core")->warn(
                    "[BoardManager] Progress Board in {} was ill-formed",
                    filename);
                break;
            } catch (std::runtime_error& err) {
                // File has been deleted at the time for reading, delete the
                // entry as well
                remove_board_signal.emit(*it);
                m_local_boards.erase(it);
                break;
            }
        }
    }
    return nullptr;
}

const std::vector<LocalBoard>& BoardManager::local_boards() const {
    if (!m_loaded) {
        throw std::logic_error{"Board is not valid"};
    }
    return m_local_boards;
}

std::string BoardManager::local_add(const std::string& name,
                                    const std::string& background) {
    Board board{name, background};
    const std::string board_filename = gen_filename(BOARD_DIR, board);
    LocalBoard local_board{board_filename, std::make_shared<Board>(board),
                           false};
    __local_save(local_board);

    m_local_boards.push_back(local_board);
    add_board_signal.emit(local_board);

    return board_filename;
}

void BoardManager::local_remove(const std::shared_ptr<Board>& board) {
    for (auto it = m_local_boards.begin(); it != m_local_boards.end(); it++) {
        LocalBoard local_board = *it;
        if (*(local_board.board) == *board) {
            m_local_boards.erase(it);
            fs::remove(local_board.filename);
            remove_board_signal.emit(local_board);
            return;
        }
    }
}

void BoardManager::local_save(const std::shared_ptr<Board>& board) {
    for (auto it = m_local_boards.begin(); it != m_local_boards.end(); it++) {
        LocalBoard local_board = *it;
        if (*(local_board.board) == *board) {
            __local_save(local_board);
            save_board_signal.emit(local_board);
            return;
        }
    }
}

void BoardManager::local_close(const std::shared_ptr<Board>& board) {
    for (auto it = m_local_boards.begin(); it != m_local_boards.end(); it++) {
        if (*(it->board) == *board) {
            (*it).is_open = false;
            (*it).board->container().get_data().clear();
            return;
        }
    }
}

bool BoardManager::loaded() const { return m_loaded; }

sigc::signal<void(LocalBoard)>& BoardManager::signal_add_board() {
    return add_board_signal;
}

sigc::signal<void(LocalBoard)>& BoardManager::signal_remove_board() {
    return remove_board_signal;
}

sigc::signal<void(LocalBoard)>& BoardManager::signal_save_board() {
    return save_board_signal;
}

void BoardManager::__local_save(const LocalBoard& local) {
    auto doc = std::make_unique<tinyxml2::XMLDocument>();

    std::shared_ptr<Board> board = local.board;

    tinyxml2::XMLElement* board_element = doc->NewElement("board");
    board_element->SetAttribute("name", board->get_name().c_str());
    board_element->SetAttribute("background", board->get_background().c_str());
    board_element->SetAttribute("uuid", std::string(board->get_id()).c_str());
    doc->InsertEndChild(board_element);

    for (const auto& cardlist : board->container()) {
        tinyxml2::XMLElement* list_element = doc->NewElement("list");
        list_element->SetAttribute("name", cardlist->get_name().c_str());
        list_element->SetAttribute("uuid", cardlist->get_id().str().c_str());

        for (const auto& card : cardlist->container()) {
            tinyxml2::XMLElement* card_element = doc->NewElement("card");
            card_element->SetAttribute("name", card->get_name().c_str());
            card_element->SetAttribute("uuid", card->get_id().str().c_str());
            if (card->is_color_set())
                card_element->SetAttribute(
                    "color", color_to_string(card->get_color()).c_str());
            const Date date = card->get_due_date();
            if (date.ok()) {
                card_element->SetAttribute("due",
                                           std::format("{}", date).c_str());
                card_element->SetAttribute("complete", card->get_complete());
            }

            // Add tasks
            for (const auto& task : card->container()) {
                tinyxml2::XMLElement* task_element = doc->NewElement("task");
                task_element->SetAttribute("name", task->get_name().c_str());
                task_element->SetAttribute("done", task->get_done());
                task_element->SetAttribute("uuid",
                                           task->get_id().str().c_str());

                card_element->InsertEndChild(task_element);
                task->modify(false);
            }

            tinyxml2::XMLElement* notes_element = doc->NewElement("notes");
            notes_element->SetText(card->get_notes().c_str());
            card_element->InsertEndChild(notes_element);

            list_element->InsertEndChild(card_element);
            card->modify(false);
            card->container().modify(false);
        }
        board_element->InsertEndChild(list_element);
        cardlist->modify(false);
        cardlist->container().modify(false);
    }
    board->modify(false);
    board->container().modify(false);

    const std::filesystem::path p{local.filename};
    if (p.has_parent_path() && !std::filesystem::exists(p.parent_path())) {
        spdlog::get("core")->warn(
            "[BoardBackend] Parent path \"{}\" does not exist. Creating",
            p.parent_path().string());
        std::filesystem::create_directories(p.parent_path());
    }

    if (doc->SaveFile(local.filename.c_str()) == tinyxml2::XML_SUCCESS) {
        spdlog::get("core")->debug(
            "[BoardManager] BoardManager has saved Board \"{}\" to: {}",
            board->get_name(), local.filename);

        auto lm_filepath = std::chrono::clock_cast<std::chrono::system_clock,
                                                   std::chrono::file_clock>(
            std::filesystem::last_write_time(p));
        board->m_last_modified =
            std::chrono::floor<std::chrono::seconds>(lm_filepath);
    } else {
        spdlog::get("core")->error(
            "[BoardManager] Failed to save board \"{}\" to: {}",
            board->get_name(), local.filename);
    }
}

