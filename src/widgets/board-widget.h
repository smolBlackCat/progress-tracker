#pragma once

#include <app_info.h>
#include <core/board.h>
#include <gtkmm.h>

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
    static constexpr unsigned int SAVE_INTERVAL = 1000 * 10;
    BoardWidget(ProgressWindow& app_window);

    ~BoardWidget() override;

    /**
     * @brief Sets and updates the board widget.
     *
     * @param board pointer to a board object.
     * @param board_card_button pointer to a BoardCardButton object that have
     *        opened this board
     *
     * @details Essentially, what this method does is cleaning the previous
     *          settings existent within the widget, if there is one, and
     *          setting a new board to the widget. It also dynamically sets
     *          every aspect of the board: background and its cards and lists.
     */
    void set(Board* board, BoardCardButton* board_card_button);

    /**
     * @brief Cleans the BoardWidget to an empty state, that is, there will be
     *        no pointer to a board object and also the information on
     *        background and cardlist objects are also deleted.
     */
    void clear();

    /**
     * @brief Saves the contents edited in the Board class.
     *
     * @param free indicates whether to free allocated memory by Board
     */
    bool save(bool free = true);

    /**
     * @brief Adds a new CardlistWidget widget based on the CardList object.
     *
     * @param cardlist CardList object
     * @param editing_mode bool indicating whether the cardlist is completely
     * new (has not been loaded from a file) or not
     */
    ui::CardlistWidget* add_cardlist(const CardList& cardlist,
                                     bool editing_mode = false);

    /**
     * @brief Removes a CardlistWidget widget.
     *
     * @param cardlist reference to the cardlist to be removed.
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
     * @param background string referring to a background, either of
     * "colour" or "file" or even "invalid"
     * @param modify boolean indicating whether the inner board object will
     * count this operation as a modfication. Default is true
     */
    void set_background(const std::string& background, bool modify = true);

    /**
     * @brief Retrieves the background string
     */
    std::string get_background();

    /**
     * @brief Updates board's name, reflecting those changes to the application
     * window
     *
     * @param board_name new name for the board
     */
    void set_board_name(const std::string& board_name);

    /**
     * @brief Retrieves the board's name
     */
    std::string get_board_name();

    /**
     * @brief Sets a new filepath from where the current board object will be
     * loaded from
     *
     * @param board_filepath path to a board file
     */
    void set_filepath(const std::string& board_filepath);

    /**
     * @brief Retrieves the current board file path
     */
    std::string get_filepath();

    bool on_drag;

private:
#ifdef WINDOWS
    Gtk::Overlay overlay;
    Gtk::Picture picture;
    Gtk::ScrolledWindow scr;
#endif
    Gtk::Box root;
    Gtk::Button add_button;
    Board* board = nullptr;
    BoardCardButton* board_card_button = nullptr;
    Glib::RefPtr<Gtk::CssProvider> css_provider_refptr;
    std::vector<ui::CardlistWidget*> cardlist_vector;
    ProgressWindow& app_window;
    double x, y;

    /**
     * @brief Sets up automatic scrolling for every time the users drags either
     * cards or cardlists across the screen the BoardWidget will scroll as
     * needed.
     */
    void setup_auto_scrolling();
};
}  // namespace ui
