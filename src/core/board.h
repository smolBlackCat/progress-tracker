#pragma once

#include <tinyxml2.h>

#include <string>

#include "cardlist.h"
#include "item-container.h"
#include "item.h"
#include "modifiable.h"

enum class BackgroundType { COLOR, IMAGE, INVALID };

using namespace std::chrono;

class BoardManager;

/**
 * @brief Kanban Board
 */
class Board : public Item, public Modifiable {
public:
    /**
     * @brief Returns the type of a given background
     *
     * @returns a BackgroundType enum informing the background's type
     */
    static BackgroundType get_background_type(const std::string& background);

    // Black colour
    static const std::string BACKGROUND_DEFAULT;

    friend BoardManager;
    friend std::shared_ptr<Board> unitialized_board(const std::string& filename);

    Board() = delete;
    Board(const std::string& name, const std::string& background);
    Board(const std::string& name, const std::string& background,
          const xg::Guid uuid);

    ~Board();

    void set_name(const std::string& name) override;

    /**
     * @brief Sets the board background image or colour
     */
    void set_background(const std::string& image_filename);
    void set_background(const Color& color);

    /**
     * @brief Returns the current background value
     *
     * @returns The background value
     */
    std::string get_background() const;

    /**
     * @brief Get last modification time in seconds
     */
    time_point<system_clock, seconds> get_last_modified() const;

    /**
     * @brief Returns true if either the board data or the board's container has
     * been modified
     */
    bool modified() const override;

    /**
     * @brief Sets board as modified */
    void modify(bool m = true) override;

    /**
     * @brief Returns the container of the board
     *
     * @returns The container reference of the board
     */
    ItemContainer<CardList>& container();

    sigc::signal<void(std::string)>& signal_background();
    sigc::signal<void(std::string)>& signal_description();

protected:
    std::string m_background, m_description;
    time_point<system_clock, seconds> m_last_modified;
    ItemContainer<CardList> m_cardlists;

    bool m_modified = false;

    // Signals
    sigc::signal<void(std::string)> m_background_signal;
    sigc::signal<void(std::string)> m_description_signal;
};
