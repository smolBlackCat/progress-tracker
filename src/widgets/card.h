#pragma once

#include <core/card.h>
#include <gtkmm.h>

#include <memory>

#include "cardlist-widget.h"

namespace ui {

class CardlistWidget;

/**
 * @brief Card widget
 */
class CardWidget : public Gtk::Box {
public:
    /**
     * @brief CardWidget constructor
     *
     * @param card a smart pointer pointing to a Card object.
     * @param is_new flag indicating whether this object is being created from
     * scratch rather than being loaded from a Progress board file. True means
     * the Card did not come from a file otherwise False
     */
    CardWidget(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               std::shared_ptr<Card> card, bool is_new = false);

    void set_title(const std::string& label);

    std::string get_title() const;

    /**
     * @brief Removes itself from the associated CardlistWidget object.
     */
    void remove_from_parent();

    /**
     * @brief Sets a new parent to this card.
     *
     * @param cardlist_p pointer to a new CardlistWidget object (parent)
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
     * @brief Gets the cardlist widget from which this widget is associated with
     */
    CardlistWidget const* get_cardlist_widget() const;

    /**
     * @brief Updates label informing the amount of tasks complete
     */
    void update_complete_tasks();

    /**
     * @brief Updates label informing due date for this card
     */
    void update_due_date();

    /**
     * @brief Changes due date label colour depending on the card's
     * situation. Red if it is past due date, green if it is complete and plain
     * colour if it is due but not yet complete
     */
    void update_due_date_label_style();

    /**
     * @brief Changes the complete tasks label colour depending on the amount of
     * tasks complete. Green if all tasks are complete, Yellow if half of the
     * tasks are complete and Red if less than half of the task are complete
     */
    void update_complete_tasks_style(unsigned long n_complete_tasks);

    /**
     * @brief Helper method for creating a tooltip text for the card widget
     */
    std::string create_details_text() const;

protected:
    bool is_new;
    Gtk::Revealer *card_cover_revealer, *card_entry_revealer;
    Gtk::Picture* card_cover_picture;
    Gtk::Label *card_label, *complete_tasks_label, *due_date_label;
    Gtk::Entry* card_entry;
    Gtk::MenuButton* card_menu_button;
    Gtk::PopoverMenu popover_menu;

    Glib::RefPtr<Gtk::EventControllerKey> key_controller;
    Glib::RefPtr<Gtk::GestureClick> card_label_click_controller,
        click_controller;
    Glib::RefPtr<Gtk::EventControllerFocus> focus_controller;
    Glib::RefPtr<Gio::MenuModel> card_menu_model;
    Glib::RefPtr<Gtk::ColorDialog> color_dialog;

    ui::CardlistWidget* cardlist_p;
    std::shared_ptr<Card> card;

    std::string last_complete_tasks_label_css_class = "";
    std::string last_due_date_label_css_class = "due-date";

    void setup_drag_and_drop();
    void open_color_dialog();
    void open_card_details_dialog();
    void on_rename();
    void off_rename();
    void on_confirm_changes();
    void on_cancel_changes();
    void clear_color();
};
}  // namespace ui
