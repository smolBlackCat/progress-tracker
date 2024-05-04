#pragma once

#include <gtkmm.h>

#include <memory>
#include <thread>
#include <vector>

#include "../core/board.h"
#include "board-card-button.h"

#define CSS_FORMAT \
    "#board-root {transition-property: background-image, background-color;}"
#define CSS_FORMAT_FILE                                                       \
    "#board-root {{transition-property: background-image, background-color; " \
    "background-size: cover;"                                                 \
    "background-repeat: no-repeat;"                                           \
    "background-image: url(file:{});}}"
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
     * @brief Adds a new CardlistWidget widget based on a given smart pointer
     * pointing to a CardList object.
     *
     * @param cardlist_ptr smart pointer pointing to the cardlist.
     * @param is_new bool indicating whether the cardlist is completely new (has
     *               not been loaded from a file) or not
     */
    void add_cardlist(std::shared_ptr<CardList> cardlist_ptr,
                      bool is_new = false);

    /**
     * @brief Removes a CardlistWidget widget.
     *
     * @param cardlist reference to the cardlist to be removed.
     */
    bool remove_cardlist(ui::CardlistWidget& cardlist);

    /**
     * @brief Sets the Board background
     *
     * @param background string referring to a background, either of
     * "colour" or "file" or even "invalid"
     */
    void set_background(const std::string& background);

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

    bool on_drag = false;

private:
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

    /**
     * @brief Sets up drag and drop system of the board
     */
    void setup_drag_and_drop(ui::CardlistWidget* new_cardlist);
};
}  // namespace ui