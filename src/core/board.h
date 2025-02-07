#pragma once

#include <tinyxml2.h>

#include <map>
#include <string>

#include "cardlist.h"
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
    Board() = delete;
    Board(const std::string& name, const std::string& background,
          const BoardBackend& backend);
    Board(const std::string& name, const std::string& background,
          const xg::Guid uuid, const BoardBackend& backend);
    Board(BoardBackend& backend);

    ~Board();

    friend BoardBackend;

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
     * @brief Adds a CardList object to the board by copying the contents to a
     * dynamic allocated space. Repeated exact same Cardlists are not allowed
     *
     * @param cardlist CardList object
     *
     * @returns a CardList pointer to the newly allocated object. nullptr may be
     * returned if the cardlist to be added is already in cardlists
     */
    std::shared_ptr<CardList> add(const CardList& cardlist);

    /**
     * @brief Removes a CardList object from the board and free the allocated
     * space linked to the cardlist object.
     *
     * @returns True if the CardList object is removed from the board.
     *          False is returned if the CardList object requested to be
     *          removed isn't in the board.
     */
    bool remove(const CardList& cardlist);

    /**
     * @brief Reorders cardlist "next" after cardlist "sibling"
     */
    void reorder(const CardList& next, const CardList& sibling);

    /**
     * @brief Saves the board using the backend functionality
     */
    bool save();

    /**
     * @brief Load the remaining data (cardlists)
     */
    void load();

    /**
     * @brief Access the underlying cardlists collection
     */
    const std::vector<std::shared_ptr<CardList>>& get_cardlists();

    void set_modified(bool modified) override;

    time_point<system_clock, seconds> get_last_modified() const;

    /**
     * @brief Returns true if the board was modified in some way, otherwise
     * False.
     *
     * @returns Boolean indicating if the board was modified.
     */
    bool get_modified() override;

    /**
     * @brief Returns true if the cardlists are loaded, otherwise false.
     */
    bool is_loaded();

    /**
     * @brief Returns the type of a given background
     *
     * @returns a BackgroundType enum informing the background's type
     */
    static BackgroundType get_background_type(const std::string& background);

    static const std::string BACKGROUND_DEFAULT;

    BoardBackend backend;

protected:
    std::string background;
    std::vector<std::shared_ptr<CardList>> cardlists;
    time_point<system_clock, seconds> last_modified;

    bool fully_loaded = false;
};