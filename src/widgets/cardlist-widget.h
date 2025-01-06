#pragma once

#include <core/card.h>
#include <core/cardlist.h>
#include <gtkmm.h>

#include <memory>

#include "board-widget.h"
#include "card.h"
#include "editable-label-header.h"

namespace ui {

class BoardWidget;
class CardWidget;

/**
 * @brief Class that implements the facilities of a card list widget.
 */
class CardlistWidget : public Gtk::ListBox {
public:
    static constexpr int CARDLIST_MAX_WIDTH =
        240;  ///< Maximum width for the card list widget.

    /**
     * @brief Constructs a CardlistWidget object.
     *
     * @param board BoardWidget reference to where this widget belongs.
     * @param cardlist CardList smart pointer that this widget is allowed
     *        to change.
     * @param is_new Indicates whether it's completely new, therefore giving the
     *        user the chance to cancel creation.
     */
    CardlistWidget(BoardWidget& board, std::shared_ptr<CardList> cardlist,
                   bool is_new = false);

    /**
     * @brief Destroys the CardlistWidget object.
     */
    ~CardlistWidget() override;

    /**
     * @brief Adds a CardWidget object based on the given Card.
     *
     * @param card Card object reference.
     * @param editing_mode Boolean indicating whether the card widget should
     * start in editing mode. Default is false.
     *
     * @return The created CardWidget object pointer.
     */
    ui::CardWidget* add(const Card& card, bool editing_mode = false);

    /**
     * @brief Removes the specified CardWidget.
     *
     * @param card Pointer to the CardWidget to be deleted.
     */
    void remove(ui::CardWidget* card);

    /**
     * @brief Retrieves the underlying CardList smart pointer.
     *
     * @return Reference to the CardList smart pointer.
     */
    const std::shared_ptr<CardList>& get_cardlist();

    /**
     * @brief Determines whether a given CardWidget object belongs to this
     *        CardlistWidget instance.
     *
     * @param card Pointer to the CardWidget object to check.
     *
     * @return True if the CardWidget belongs to this CardlistWidget,
     *         false otherwise.
     */
    bool is_child(ui::CardWidget* card);

    /**
     * @brief Reorders card widget "next" after card widget "sibling".
     *
     * @param next Reference to the CardWidget to be placed after the sibling.
     * @param sibling Reference to the CardWidget to be placed before the next.
     */
    void reorder(ui::CardWidget& next, ui::CardWidget& sibling);

    /**
     * @brief Access the card widgets tracker vector.
     *
     * @return Reference to the vector of CardWidget pointers.
     */
    const std::vector<ui::CardWidget*>& get_card_widgets();

    BoardWidget& board;  ///< Reference to the BoardWidget to which this
                         ///< CardlistWidget belongs.

private:
    /**
     * @brief Sets up drag and drop functionality for the card list widget.
     */
    void setup_drag_and_drop();

    // Widgets
    EditableLabelHeader cardlist_header;  ///< Header widget for the card list.
    Gtk::Button add_card_button;          ///< Button to add new cards.
    Gtk::Box root;  ///< Root container for the card list widget.

    // Data
    std::shared_ptr<CardList>
        cardlist;  ///< Pointer to the current CardList object.
    std::vector<ui::CardWidget*>
        card_widgets;  ///< Vector holding pointers to CardWidget objects.

    bool is_new;  ///< Flag indicating whether the card list is new or loaded
                  ///< from a file.
};

}  // namespace ui