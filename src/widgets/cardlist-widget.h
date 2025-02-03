#pragma once

#include <core/card.h>
#include <core/cardlist.h>
#include <glibmm/extraclassinit.h>
#include <gtkmm.h>

#include <memory>

#include "board-widget.h"
#include "card.h"
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
class CardlistWidget : public CardlistInit, public Gtk::Widget {
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

    CardWidget* _add(const std::shared_ptr<Card>& card,
                     bool editing_mode = false);

    /**
     * @brief Retrieves the number of visible children in the root box.
     *
     * @return the number of visible children.
     */
    int get_n_visible_children() const;

    /**
     * @brief Retrieves the size request mode.
     *
     * @return the size request mode.
     */
    Gtk::SizeRequestMode get_request_mode_vfunc();

    /**
     * @brief Measures the widget size.
     *
     * @param orientation the orientation to measure.
     * @param for_size the size to measure for.
     * @param minimum the minimum size.
     * @param natural the natural size.
     * @param minimum_baseline the minimum baseline.
     * @param natural_baseline the natural baseline.
     */
    void measure_vfunc(Gtk::Orientation orientation, int for_size, int& minimum,
                       int& natural, int& minimum_baseline,
                       int& natural_baseline) const override;

    /**
     * @brief Allocates size for the widget.
     *
     * @param width the allocated width.
     * @param height the allocated height.
     * @param baseline the allocated baseline.
     */
    void size_allocate_vfunc(int width, int height, int baseline) override;

    // Widgets
    EditableLabelHeader cardlist_header;  ///< Header widget for the card list.
    Gtk::ScrolledWindow scr_window{};
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