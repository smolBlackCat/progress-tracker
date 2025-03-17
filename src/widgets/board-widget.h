#pragma once

#include <core/board.h>
#include <gtkmm.h>

#include <memory>
#include <vector>

#include "board-card-button.h"

#define CSS_FORMAT \
    "#board-root {transition-property: background-image, background-color;}"
#define CSS_FORMAT_FILE                                                       \
    "#board-root {{transition-property: background-image, background-color; " \
    "background-size: cover;"                                                 \
    "background-repeat: no-repeat;"                                           \
    "background-image: url(\"file:{}\");}}"
#define CSS_FORMAT_RGB                                                        \
    "#board-root {{transition-property: background-image, background-color; " \
    "background-color: {};}}"

namespace ui {

class ProgressWindow;
class CardlistWidget;

/**
 * @brief Widget that holds a list of CardLists
 */
class BoardWidget : public Gtk::ScrolledWindow {
public:
    static constexpr unsigned int SAVE_INTERVAL =
        1000 * 10;  ///< Interval for auto-saving the board in milliseconds
    static constexpr int SCROLL_SPEED_FACTOR =
        6;  ///< Factor to control the scroll speed

    /**
     * @brief Constructs a BoardWidget object.
     */
    BoardWidget();

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
     *          every aspect of the board: background and its cards and lists.
     */
    void set(std::shared_ptr<Board>& board, BoardCardButton* board_card_button);

    /**
     * @brief Cleans the BoardWidget to an empty state, that is, there will be
     *        no pointer to a board object and also the information on
     *        background and cardlist objects are also deleted.
     */
    void clear();

    /**
     * @brief Saves the contents edited in the Board class.
     *
     * @param free indicates whether to clear the board after saving
     * @return true if the save operation was successful, false otherwise
     */
    bool save(bool free = true);

    /**
     * @brief Adds a new CardlistWidget widget based on the CardList object.
     *
     * @param cardlist CardList object
     * @param editing_mode bool indicating whether the cardlist is completely
     * new (has not been loaded from a file) or not
     * @return pointer to the newly added CardlistWidget
     */
    ui::CardlistWidget* add_cardlist(const CardList& cardlist,
                                     bool editing_mode = false);

    /**
     * @brief Removes a CardlistWidget widget.
     *
     * @param cardlist reference to the cardlist to be removed.
     * @return true if the cardlist was successfully removed, false otherwise
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
     * @param background string referring to a background, either a colour code
     * or a filename
     * @param modify boolean indicating whether the inner board object will
     * count this operation as a modification. Default is true
     */
    void set_background(const std::string& background, bool modify = true);

    /**
     * @brief Retrieves the background string
     *
     * @return reference to the background string
     */
    const std::string& get_background() const;

    /**
     * @brief Updates board's name, reflecting those changes to the application
     * window
     *
     * @param board_name new name for the board
     */
    void set_name(const std::string& board_name);

    /**
     * @brief Retrieves the board's name
     *
     * @return reference to the board's name string
     */
    const std::string& get_name() const;

    /**
     * @brief Returns true if the board is set up to horizontally scroll
     *
     * @return true if horizontal scrolling is enabled, false otherwise
     */
    bool get_on_scroll() const;

    /**
     * @brief Describes whether the board should be able to scroll horizontally
     *
     * @param scroll boolean indicating whether horizontal scrolling should be
     * enabled
     */
    void set_on_scroll(bool scroll = true);

    /**
     * @brief Retrieves the current board object
     *
     * @return shared pointer to the current board object
     */
    std::shared_ptr<Board> get_board() const;

private:
    /**
     * @brief Sets up automatic scrolling for every time the users drags either
     * cards or cardlists across the screen the BoardWidget will scroll as
     * needed.
     */
    void setup_auto_scrolling();

    /**
     * @brief Adds a new cardlist widget to the board widget.
     */
    CardlistWidget* _add_cardlist(const std::shared_ptr<CardList>& cardlist,
                                  bool editing_mode = false);

#ifdef WIN32
    Gtk::Overlay overlay;     ///< Overlay widget for Windows platform
    Gtk::Picture picture;     ///< Picture widget for Windows platform
    Gtk::ScrolledWindow scr;  ///< ScrolledWindow widget for Windows platform
#endif

    Gtk::Box root;           ///< Root container for the board widget
    Gtk::Button add_button;  ///< Button to add new card lists
    std::shared_ptr<Board> board =
        nullptr;  ///< Pointer to the current board object
    BoardCardButton* board_card_button =
        nullptr;  ///< Pointer to the BoardCardButton that opened this board
    Glib::RefPtr<Gtk::CssProvider>
        css_provider_refptr;  ///< CSS provider for styling the widget
    std::vector<ui::CardlistWidget*>
        cardlist_widgets;  ///< Vector holding pointers to CardlistWidget objects
    double x, y;          ///< Cursor Position
    bool on_scroll =
        false;  ///< Flag indicating whether horizontal scrolling is enabled
};

}  // namespace ui