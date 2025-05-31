#pragma once

#include <core/card.h>
#include <core/cardlist.h>
#include <glibmm/extraclassinit.h>
#include <gtkmm.h>

#include "base-item.h"
#include "board-widget.h"
#include "card-widget.h"
#include "editable-label-header.h"

extern "C" {
static void cardlist_class_init(void* klass, void* user_data);
}

namespace ui {

class BoardWidget;
class CardWidget;

class CardlistInit : public Glib::ExtraClassInit {
public:
    CardlistInit();
};

/**
 * @brief Class that implements the facilities of a card list widget.
 */
class CardlistWidget : public CardlistInit, public BaseItem {
public:
    static constexpr int CARDLIST_MAX_WIDTH = 240;

    /**
     * @brief Constructs a CardlistWidget object.
     *
     * @param board BoardWidget reference to where this widget belongs.
     * @param cardlist CardList smart pointer that this widget is allowed
     *        to change.
     * @param editing_mode Boolean indicating whether the card list widget
     * should start in editing mode. Default is false.
     */
    CardlistWidget(BoardWidget& board,
                   const std::shared_ptr<CardList>& cardlist,
                   bool editing_mode = false);

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
    void remove(ui::CardWidget& card);

    /**
     * @brief Retrieves the underlying CardList smart pointer.
     *
     * @return Reference to the CardList smart pointer.
     */
    const std::shared_ptr<CardList>& cardlist();

    /**
     * @brief Determines whether a given CardWidget object belongs to this
     *        CardlistWidget instance.
     *
     * @param card Pointer to the CardWidget object to check.
     *
     * @return True if the CardWidget belongs to this CardlistWidget,
     *         false otherwise.
     */
    bool is_child(ui::CardWidget& card);

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
    const std::vector<ui::CardWidget*>& cards();

    BoardWidget& board;

protected:
    /**
     * @brief Sets up drag and drop functionality for the card list widget.
     */
    void setup_drag_and_drop();

    CardWidget* _add(const std::shared_ptr<Card>& card,
                     bool editing_mode = false);

    void cleanup() override;

    // Widgets
    EditableLabelHeader header;
    Gtk::ScrolledWindow scr_window{};
    Gtk::Button add_card_button;
    Gtk::Box root;

    // Data
    std::shared_ptr<CardList> m_cardlist;
    std::vector<ui::CardWidget*> m_cards;

    bool is_new;
};

}  // namespace ui