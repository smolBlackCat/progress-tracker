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
 * @brief Cardlist Widget
 */
class CardlistWidget : public CardlistInit, public BaseItem {
public:
    static constexpr int CARDLIST_MAX_WIDTH = 240;

    /**
     * @brief CardlistWidget constructor
     *
     * @param parent_board BoardWidget reference
     * @param cardlist CardList data
     * @param editing_mode Boolean indicating whether the card list widget
     * should start in editing mode. Default is false.
     */
    CardlistWidget(BoardWidget& parent_board,
                   const std::shared_ptr<CardList>& cardlist,
                   bool editing_mode = false);

    /**
     * @brief Reorders card widget "next" after card widget "sibling".
     *
     * @param next Reference to the CardWidget to be placed after the sibling.
     * @param sibling Reference to the CardWidget to be placed before the next.
     */
    void reorder(ui::CardWidget& next, ui::CardWidget& sibling);

    /**
     * @brief Removes CardWidget instance
     *
     * @param card CardWidget reference to remove
     */
    void remove(ui::CardWidget& card);

    /**
     * @brief Adds a new Card instance to this object's CardList instance and
     * then adds the respective CardWidget
     *
     * @param card Card object reference.
     * @param editing_mode bool value indicating whether the newly created
     * widget instance should begin in editing mode
     *
     * @return CardWidget instance pointer pointing to the newly added instance.
     */
    ui::CardWidget* add(const Card& card, bool editing_mode = false);

    /**
     * @brief Determines whether this CardlistWidget instance is the given
     * CardWidget instance's parent
     *
     * @param card CardWidget instance reference
     *
     * @return True if this instance is card's parent, otherwise false
     */
    bool is_child(ui::CardWidget& card);

    /**
     * @brief Access the card widgets tracker vector.
     *
     * @return Reference to the vector of CardWidget pointers.
     */
    const std::vector<ui::CardWidget*>& cards();

    /**
     * @brief Retrieves the underlying CardList smart pointer.
     *
     * @return Reference to the CardList smart pointer.
     */
    const std::shared_ptr<CardList>& cardlist();

    BoardWidget& board;

protected:
    /**
     * @brief Sets up drag and drop functionality for the card list widget.
     */
    void setup_drag_and_drop();

    void cleanup() override;

    CardWidget* __add(const std::shared_ptr<Card>& card,
                     bool editing_mode = false);

    // Widgets
    EditableLabelHeader m_header;
    Gtk::ScrolledWindow m_scr_window;
    Gtk::Button m_add_card_button;
    Gtk::Box m_root;

    // Data
    std::shared_ptr<CardList> m_cardlist;
    std::vector<ui::CardWidget*> m_cards;

    bool m_new;
};

}  // namespace ui