#pragma once

#include <core/card.h>
#include <gtkmm.h>

#include <memory>

#include "cardlist-widget.h"
#include "editable-label-header.h"

namespace ui {

class CardlistWidget;

/**
 * @brief Card widget
 */
class CardWidget : public EditableLabelHeader {
public:
    static constexpr int COLOR_FRAME_HEIGHT = 30;

    /**
     * @brief CardWidget constructor
     *
     * @param card_ptr a smart pointer pointing to a Card object.
     * @param is_new flag indicating whether this object is being created from
     * scratch rather than being loaded from a Progress board file. True means
     * the Card did not come from a file otherwise False
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

    /**
     * @brief Sets this card's colour
     *
     * @param color rgba object representing the colour
     */
    void set_color(const Gdk::RGBA& color);

    /**
     * @brief Get the underlying Card object from which this widget represents
     *
     * @return pointer to the Card object
     */
    std::shared_ptr<Card> get_card();

    /**
     * @brief Updates card's progress bar
     *
     */
    void update_completed();

    /**
     * @brief Hides the card's progress bar
     *
     * @param hide bool indicating whether to hide the progress bar or not.
     * Default is true
     */
    void hide_progress_bar(bool hide = true);

protected:
    bool is_new;
    Gtk::Picture color_frame;
    Gtk::Frame m_frame;
    Gtk::Button clear_colour_button;
    Glib::RefPtr<Gtk::ColorDialog> color_dialog;
    Gtk::Box colour_setting_box;
    Gtk::ProgressBar progress_bar;

    void setup_drag_and_drop();
    void open_color_dialog();
    void clear_color();

private:
    std::shared_ptr<Card> card_refptr;
    ui::CardlistWidget* cardlist_p;
};
}  // namespace ui

