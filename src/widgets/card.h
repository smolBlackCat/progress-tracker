#pragma once

#include <gtkmm.h>

#include <memory>
#include <string>

#include "../core/card.h"
#include "cardlist-widget.h"
#include "editable-label-header.h"

namespace ui {

class CardlistWidget;

/**
 * @brief Card widget that allows the user to make some modifications like
 * renaming and removing.
 */
class CardWidget : public EditableLabelHeader {
public:
    /**
     * @brief CardWidget constructor
     *
     * @param card_ptr a smart pointer pointing to a Card object.
     */
    CardWidget(std::shared_ptr<Card> card_refptr, bool is_new = false);

    /**
     * @brief Removes itself from the associated CardlistWidget object.
     */
    void remove_from_parent();

    /**
     * @brief Sets a new parent to this card.
     *
     * @param cardlist_p pointer to a new CardlistWidget object (parent)
     *
     * @details Cards in Progress Tracker are the only ones that changes parents
     *          with a certain frequency, that's why this method exists.
     */
    void set_cardlist(ui::CardlistWidget* cardlist_p);

    void set_color(const Gdk::RGBA& color);

    std::shared_ptr<Card> get_card();

protected:
    bool is_new;
    Gtk::Picture color_frame;
    Gtk::Frame m_frame;
    Gtk::Button clear_colour_button;
    Gtk::ColorDialogButton colour_selector_button;
    Glib::RefPtr<Gtk::ColorDialog> color_dialog;
    Gtk::Box colour_setting_box;

    void setup_drag_and_drop();
    void open_color_dialog();

private:
    std::shared_ptr<Card> card_refptr;
    ui::CardlistWidget* cardlist_p;
};
}  // namespace ui