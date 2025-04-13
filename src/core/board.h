#pragma once

#include <tinyxml2.h>

#include <map>
#include <string>

#include "cardlist.h"
#include "item-container.h"
#include "item.h"

enum class BackgroundType { COLOR, IMAGE, INVALID };

enum class BackendType { LOCAL, CALDAV, NEXTCLOUD };

class Board;

using namespace std::chrono;

/**
 * @brief Helper class for dealing with multiple ways of storing and creating
 * Board objects
 */
class BoardBackend {
public:
    /**
     * @brief BoardBackend constructor
     *
     * @param backend_type backend to use for saving and storing
     * @param settings backend settings. The settings used are backend type
     * reliant*/
    BoardBackend(BackendType backend_type,
                 const std::map<std::string, std::string>& settings = {});

    BoardBackend() = delete;

    /**
     * @brief Loads a board object using this backend. This method call
     * assumes the backend has the appropriate settings to load a Board object
     */
    Board load();

    /**
     * @brief Creates a new Board object using this backend
     */
    Board create(const std::string& name, const std::string& background);

    /**
     * @brief Returns the attribute key. May be empty if the key is not
     * available for the current backend type.
     */
    std::string get_attribute(const std::string& key) const;

    /**
     * @brief Assigns a value to a key. Nothing is done if the key to be set
     * does not correspond to the keys available to current backend type
     */
    bool set_attribute(const std::string& key, const std::string& value);

    /**
     * @brief Save board depending on the type
     */
    bool save(Board& board);

    /**
     * @brief Load cardlists into board. The second call to this function won't
     * do anything since it is expected that the board is fully loaded
     */
    void load_cardlists(Board& board);

    BackendType get_type() const noexcept;

protected:
    bool save_xml(Board& board);
    bool save_caldav(Board& board);
    bool save_nextcloud(Board& board);

    BackendType type;
    std::map<std::string, std::string> settings;
};

/**
 * @brief Kanban Board
 */
class Board : public Item {
public:
    /**
     * @brief Returns the type of a given background
     *
     * @returns a BackgroundType enum informing the background's type
     */
    static BackgroundType get_background_type(const std::string& background);

    // Black colour
    static const std::string BACKGROUND_DEFAULT;

    friend BoardBackend;

    Board() = delete;
    Board(const std::string& name, const std::string& background,
          const BoardBackend& backend);
    Board(const std::string& name, const std::string& background,
          const xg::Guid uuid, const BoardBackend& backend);
    Board(BoardBackend& backend);

    ~Board();

    /**
     * @brief Load the remaining data (cardlists)
     */
    void load();

    // FIXME: set_background naturally modifies a board
    /**
     * @brief Changes the background information of the board. If the given
     *        background is BackgroundType::INVALID, then the background will
     *        fall back for a default defined in Board::BACKGROUND_DEFAULT.
     *
     * @param other The new background.
     * @param modify flag indicating whether to count this operation as a
     * modification
     *
     * @returns a BackgroundType for later processing. It may be ignored
     */
    BackgroundType set_background(const std::string& other, bool modify = true);

    /**
     * @brief Returns the current background value
     *
     * @returns The background value
     */
    const std::string& get_background() const;

    /**
     * @brief Get last modification time in seconds
     */
    time_point<system_clock, seconds> get_last_modified() const;

    /**
     * @brief Returns true if either the board data or the board's container has
     * been modified
     */
    bool get_modified() const override;

    /**
     * @brief Returns true if the cardlists are loaded, otherwise false.
     */
    bool is_loaded();

    /**
     * @brief Saves the board using the backend functionality
     *
     * @returns True if the board was successfully saved
     */
    bool save();

    /**
     * @brief Returns the container of the board
     *
     * @returns The container reference of the board
     */
    ItemContainer<CardList>* container();

    BoardBackend backend;

protected:
    std::string background;
    ItemContainer<CardList> cardlists;
    time_point<system_clock, seconds> last_modified;

    bool fully_loaded = false;
};