#pragma once

#include <gtkmm.h>

#include <memory>

#include "../core/card.h"
#include "../core/cardlist.h"
#include "board-widget.h"
#include "card.h"
#include "editable-label-header.h"

namespace ui {

class BoardWidget;
class CardWidget;

/**
 * @brief Class that implements the facilities of a card list widget
 */
class CardlistWidget : public Gtk::ListBox {
public:
    static constexpr int CARDLIST_SIZE = 240;

    /**
     * @brief CardlistWidget's constructor
     *
     * @param board BoardWidget reference to where this widget belongs
     * @param cardlist_refptr CardList smart pointer that this widget is allowed
     *        to change
     * @param is_new Indicates whether it's completely new, therefore giving the
     *        user the chance to cancel creation
     */
    CardlistWidget(BoardWidget& board,
                   std::shared_ptr<CardList> cardlist_refptr,
                   bool is_new = false);

    /**
     * @brief Adds a CardWidget object based on the given Card
     *
     * @param card Card object
     * @param editing_mode Boolean indicating whether the card widget should
     * start in editing mode. Default is false
     *
     * @return The created CardWidget object pointer
     */
    ui::CardWidget* add_card(const Card& card, bool editing_mode = false);

    /**
     * @brief Removes the specified CardWidget
     *
     * @param card Pointer to the CardWidget to be deleted.
     */
    void remove_card(ui::CardWidget* card);

    /**
     * @brief Retrieves the underlying CardList smart pointer.
     *
     * @return Reference to the CardList smart pointer.
     */
    std::shared_ptr<CardList>& get_cardlist_refptr();

    /**
     * @brief Determines whether a given CardWidget object belongs to this
     *        CardlistWidget instance.
     *
     * @param card Pointer to the CardWidget object to check
     *
     * @return True if the CardWidget belongs to this CardlistWidget,
     *         false otherwise.
     */
    bool is_child(ui::CardWidget* card);

    /**
     * @brief Returns CardListHeader object associated with this CardlistWidget
     *        instance.
     *
     * @return Reference to the CardListHeader object.
     */
    EditableLabelHeader& get_header();

    bool is_new;
    BoardWidget& board;

    void reorder_cardwidget(ui::CardWidget& next, ui::CardWidget& sibling);

private:
    void setup_drag_and_drop();

    // Widgets
    EditableLabelHeader cardlist_header;
    Gtk::Button add_card_button;
    Gtk::Box root;

    // Data
    std::shared_ptr<CardList> cardlist_refptr;
    std::vector<ui::CardWidget*> cards_tracker;
};
}  // namespace ui