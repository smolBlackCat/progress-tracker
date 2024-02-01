#pragma once

#include <gtkmm.h>

#include <memory>
#include <vector>

#include "../core/board.h"
#include "board-card-button.h"
#include "cardlist.h"

#define CSS_FORMAT \
    "#board-root {transition-property: background-image, background-color;}"
#define CSS_FORMAT_FILE                                                       \
    "#board-root {{transition-property: background-image, background-color; " \
    "background-size: cover;" \
    "background-repeat: no-repeat;" \
    "background-image: url(file:{});}}"
#define CSS_FORMAT_RGB                                                        \
    "#board-root {{transition-property: background-image, background-color; " \
    "background-color: {};}}"

namespace ui {

/**
 * @brief Widget that holds a list of CardLists
*/
class BoardWidget : public Gtk::ScrolledWindow {
public:
    BoardWidget();

    ~BoardWidget() override;

    /**
     * @brief Sets and updates the board widget.
     * 
     * @param board pointer to a board object.
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
     *        background and cardlists is also deleted.
    */
    void clear();

    /**
     * @brief Saves the contents edited in the Board class.
    */
    bool save();

    /**
     * @brief Adds a new Cardlist widget based on a given smart pointer pointing
     *        to a CardList object.
     * 
     * @param cardlist_ptr smart pointer pointing to the cardlist.
     */
    void add_cardlist(std::shared_ptr<CardList> cardlist_ptr);

    /**
     * @brief Removes a Cardlist widget.
     * 
     * @param cardlist reference to the cardlist to be removed.
    */
    bool remove_cardlist(ui::Cardlist& cardlist);

    ui::BoardCardButton* board_card_button;

    bool set_background();

private:
    Gtk::Box root;
    Gtk::Button add_button;
    Board* board;
    Glib::RefPtr<Gtk::CssProvider> css_provider_refptr;
    std::vector<ui::Cardlist*> cardlist_vector;
};
}  // namespace ui