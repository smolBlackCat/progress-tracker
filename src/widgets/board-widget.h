#pragma once

#include <core/board-manager.h>
#include <gtkmm.h>

#include "board-card-button.h"

namespace ui {
class CardlistWidget;

/**
 * @brief Widget that holds a list of CardLists
 */
class BoardWidget : public Gtk::ScrolledWindow {
public:
    static constexpr const char* CSS_FORMAT =
        "#board-root {transition-property: background-image, "
        "background-color;}";
    static constexpr const char* CSS_FORMAT_FILE =
        "#board-root {{transition-property: background-image, "
        "background-color; "
        "background-size: cover;"
        "background-repeat: no-repeat;"
        "background-image: url(\"file:{}\");}}";
    static constexpr const char* CSS_FORMAT_RGB =
        "#board-root {{transition-property: background-image, "
        "background-color; "
        "background-color: {};}}";

    static constexpr unsigned int SAVE_INTERVAL = 1000 * 10;
    static constexpr int SCROLL_SPEED_FACTOR = 6;

    /**
     * @brief Constructs a BoardWidget object.
     */
    BoardWidget(BoardManager& manager);

    /**
     * @brief Sets and updates the board widget.
     *
     * @param board pointer to a board object.
     * @param board_card_button pointer to a BoardCardButton object that has
     *        opened this board
     *
     * @details Essentially, what this method does is cleaning the previous
     *          settings existent within the widget, if there is one, and
     *          setting a new board to the widget. It also dynamically sets
     *          every aspect of the board: background and its cards and
     * lists.
     */
    void set(const std::shared_ptr<Board>& board,
             BoardCardButton* const board_card_button);

    /**
     * @brief Cleans the BoardWidget to an empty state, that is, there will
     * be no pointer to a board object and also the information on
     *        background and cardlist objects are also deleted.
     */
    void clear();

    /**
     * @brief Saves the contents edited in the Board class.
     *
     * @param clear indicates whether to clear the board after saving
     */
    void save(bool clear = true);

    /**
     * @brief Adds a new CardlistWidget widget based on the CardList object.
     *
     * @param cardlist CardList object
     * @param editing_mode bool indicating whether the cardlist is
     * completely new (has not been loaded from a file) or not
     * @return pointer to the newly added CardlistWidget
     */
    ui::CardlistWidget* add_cardlist(const CardList& cardlist,
                                     bool editing_mode = false);

    /**
     * @brief Removes a CardlistWidget widget.
     *
     * @param cardlist reference to the cardlist to be removed.
     * @return true if the cardlist was successfully removed, false
     * otherwise
     */
    bool remove_cardlist(ui::CardlistWidget& cardlist);

    /**
     * @brief Reorders two CardlistWidget objects.
     *
     * @param next widget to be put after sibling
     * @param sibling widget to be put before next
     */
    void reorder_cardlist(CardlistWidget& next, CardlistWidget& sibling);

    /**
     * @brief Sets the Board background
     *
     * @param background string referring to a background, either a colour
     * code or a filename
     */
    void set_background(const std::string& background);

    /**
     * @brief Retrieves the background string
     *
     * @return reference to the background string
     */
    std::string get_background() const;

    /**
     * @brief Updates board's name, reflecting those changes to the
     * application window
     *
     * @param board_name new name for the board
     */
    void set_name(const std::string& board_name);

    /**
     * @brief Retrieves the board's name
     *
     * @return reference to the board's name string
     */
    std::string get_name() const;

    /**
     * @brief Returns true if the board is set up to horizontally scroll
     *
     * @return true if horizontal scrolling is enabled, false otherwise
     */
    bool get_on_scroll() const;

    /**
     * @brief Describes whether the board should be able to scroll
     * horizontally
     *
     * @param scroll boolean indicating whether horizontal scrolling should
     * be enabled
     */
    void set_on_scroll(bool scroll = true);

    /**
     * @brief Retrieves the current board object
     *
     * @return shared pointer to the current board object
     */
    std::shared_ptr<Board> get_board() const;

protected:
    /**
     * @brief Sets up automatic scrolling for every time the users drags
     * either cards or cardlists across the screen the BoardWidget will
     * scroll as needed.
     */
    void setup_auto_scrolling();

    /**
     * @brief Adds a new cardlist widget to the board widget.
     */
    CardlistWidget* _add_cardlist(const std::shared_ptr<CardList>& cardlist,
                                  bool editing_mode = false);

    void __set_background(const std::string& background);

    BoardManager& m_manager;
    std::vector<sigc::connection> m_connections;

#ifdef WIN32
    Gtk::Overlay overlay;
    Gtk::Picture picture;
    Gtk::ScrolledWindow scr;
#endif
    Gtk::Box root;
    Gtk::Button add_button;
    std::shared_ptr<Board> board = nullptr;
    BoardCardButton* board_card_button = nullptr;
    Glib::RefPtr<Gtk::CssProvider> css_provider_refptr;
    std::vector<ui::CardlistWidget*> cardlist_widgets;
    double x, y;
    bool on_scroll = false;
};

}  // namespace ui