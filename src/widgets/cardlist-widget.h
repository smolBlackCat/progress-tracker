#pragma once

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
     *
     * @param title cardlist's title
     */
    CardlistWidget(BoardWidget& parent_board, const std::string& title);

    void set_title(const std::string& title);

    /**
     * @brief Reorders card widget "next" after card widget "sibling".
     *
     * @param next Reference to the CardWidget to be placed after the sibling.
     * @param sibling Reference to the CardWidget to be placed before the next.
     */
    void reorder(ui::CardWidget& next, ui::CardWidget& sibling);

    void remove(CardWidget& card);

    /**
     * @brief Adds a card widget to the end of the cardlist widget. This method
     * will not do anything if the card has a parent
     *
     * @param card card widget
     */
    void append(CardWidget& card);
    void insert_after(CardWidget& card, CardWidget& sibling);

    /**
     * @brief Adds a card widget to the end of the cardlist widget. This method
     * differs from append in that it accepts cards with their defined parents
     */
    void receive(CardWidget& card);

    /**
     * @brief Adds a card from another cardlist after another card in this
     * cardlist. The visitor card must necessarily belong to another cardlist
     *
     * @param card visitor card
     * @param sibling card which visitor card is placed after
     */
    void receive_after(CardWidget& card, CardWidget& sibling);

    const std::string& get_name() const;

    bool is_child(ui::CardWidget& card);

    sigc::signal<void(std::string, std::string)>& signal_name_changed();
    sigc::signal<void(CardWidget*, int)>& signal_card_added();
    sigc::signal<void(CardWidget*)>& signal_card_removed();
    sigc::signal<void(CardWidget*, CardWidget*, bool)>& signal_card_reorder();
    sigc::signal<void(CardWidget*, CardlistWidget*, CardWidget*)>&
    signal_card_received();

    BoardWidget& board;

protected:
    void setup_drag_and_drop();
    void cleanup() override;

    // Widgets
    EditableLabelHeader m_header;
    Gtk::ScrolledWindow m_scr_window;
    Gtk::Button m_add_card_button;
    Gtk::PopoverMenu m_popover;
    Gtk::Box m_root;

    // Signals
    sigc::signal<void(std::string, std::string)> m_name_changed_signal;

    sigc::signal<void(CardWidget*, int)> m_card_add_signal;
    sigc::signal<void(CardWidget*)> m_card_remove_signal;

    /**
     * void(next, sibling, up)
     *
     * `up` is a bit flag that indicates whether the next is put before or after
     * the sibling. This bit is true if and only if next is placed before
     * sibling, otherwise false.*/
    sigc::signal<void(CardWidget*, CardWidget*, bool)> m_card_reorder_signal;

    // void(incoming_card, incoming_from, sibling)
    sigc::signal<void(CardWidget*, CardlistWidget*, CardWidget*)>
        m_card_received_signal;

    // Data
    std::string m_name;
};
}  // namespace ui
