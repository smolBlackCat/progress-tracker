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
class CardlistWidget;

/**
 * @brief Represents a header for a CardList that allows changing its name.
 */
class CardListHeader : public EditableLabelHeader {
public:
    CardListHeader(CardlistWidget& cardlist_widget);

protected:
    CardlistWidget& cardlist_widget;

    void on_confirm_changes() override;
    void on_cancel_changes() override;
};

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
     * @param card_refptr Card smart pointer
     * @param is_new bool value indicating whether this card is completely new
     *               or loaded from file.
     *
     * @return The created CardWidget object pointer
     */
    ui::CardWidget* add_card(std::shared_ptr<Card> card_refptr,
                             bool is_new = false);

    /**
     * @brief Removes the specified CardWidget
     *
     * @param card Pointer to the CardWidget to be deleted.
     */
    void remove_card(ui::CardWidget* card);

    /**
     * @brief Removes itself from the associated Board
     */
    void remove_();

    /**
     * @brief Sets the name of the card list !!!
     */
    void set_name_(const std::string& new_name);

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
    CardListHeader& get_header();

    bool is_new;

private:
    // Widgets
    CardListHeader cardlist_header;
    Gtk::Button add_card_button;
    Gtk::Box root;

    // Data
    BoardWidget& board;
    std::shared_ptr<CardList> cardlist_refptr;
    std::vector<ui::CardWidget*> cards_tracker;

    void reorder_card(ui::CardWidget& next, ui::CardWidget& sibling);

    void setup_drag_and_drop(ui::CardWidget* card);
};
}  // namespace ui