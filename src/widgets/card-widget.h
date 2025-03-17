#pragma once

#include <core/card.h>
#include <gtkmm.h>

#include <memory>

#include "base-item.h"
#include "cardlist-widget.h"
#include "glibmm/extraclassinit.h"

extern "C" {
static void card_class_init(void* g_class, void* data);
static void card_init(GTypeInstance* instance, void* g_class);
}

namespace ui {

class CardlistWidget;

class CardInit : public Glib::ExtraClassInit {
public:
    CardInit();
};

/**
 * @brief Widget that represents a single card.
 */
class CardWidget : public CardInit, public BaseItem {
public:
    /**
     * @brief Constructs a CardWidget object.
     *
     * @param card a smart pointer pointing to a Card object.
     * @param is_new flag indicating whether this object is being created from
     * scratch rather than being loaded from a Progress board file. True means
     * the Card did not come from a file otherwise False.
     */
    CardWidget(std::shared_ptr<Card> card, bool is_new = false);

    /**
     * @brief Sets the title of the card.
     *
     * @param label new title for the card.
     */
    void set_title(const std::string& label);

    /**
     * @brief Retrieves the title of the card.
     *
     * @return the card's title string.
     */
    std::string get_title() const;

    /**
     * @brief Removes itself from the associated CardlistWidget object.
     */
    void remove_from_parent();

    /**
     * @brief Sets a new parent to this card.
     *
     * @param cardlist_p pointer to a new CardlistWidget object (parent).
     */
    void set_cardlist(ui::CardlistWidget* new_parent);

    /**
     * @brief Sets this card cover's color.
     *
     * @param color rgba object representing the color.
     */
    void set_color(const Gdk::RGBA& color);

    /**
     * @brief Retrieves the underlying Card object from which this widget
     * represents.
     *
     * @return shared pointer to the Card object.
     */
    std::shared_ptr<Card> get_card();

    /**
     * @brief Retrieves the CardlistWidget from which this widget is associated
     * with.
     *
     * @return pointer to the associated CardlistWidget.
     */
    CardlistWidget const* get_cardlist_widget() const;

    /**
     * @brief Updates the label informing the amount of tasks complete.
     */
    void update_complete_tasks();

    /**
     * @brief Updates the label informing the due date for this card.
     */
    void update_due_date();

    /**
     * @brief Changes the due date label color depending on the card's
     * situation. Red if it is past due date, green if it is complete and plain
     * color if it is due but not yet complete.
     */
    void update_due_date_label_style();

    /**
     * @brief Changes the complete tasks label color depending on the amount of
     * tasks complete. Green if all tasks are complete, Yellow if half of the
     * tasks are complete and Red if less than half of the tasks are complete.
     *
     * @param n_complete_tasks number of completed tasks.
     */
    void update_complete_tasks_style(unsigned long n_complete_tasks);

    /**
     * @brief Helper method for creating a tooltip text for the card widget.
     *
     * @return the tooltip text string.
     */
    std::string create_details_text() const;

protected:
    bool is_new;
    Gtk::Box root_box;
    Gtk::Revealer card_cover_revealer, card_entry_revealer;
    Gtk::Picture card_cover_picture;
    Gtk::Label card_label, complete_tasks_label, due_date_label;
    Gtk::Entry card_entry;
    Gtk::MenuButton card_menu_button;
    Gtk::PopoverMenu popover_menu;

    Glib::RefPtr<Gtk::EventControllerFocus> focus_controller;
    Glib::RefPtr<Gtk::EventControllerKey> key_controller;
    Glib::RefPtr<Gtk::GestureClick> click_controller;
    Glib::RefPtr<Gio::Menu> card_menu_model;
    Glib::RefPtr<Gtk::ColorDialog> color_dialog;

    ui::CardlistWidget* parent;
    std::shared_ptr<Card> card;

    std::string last_complete_tasks_label_css_class = "";
    std::string last_due_date_label_css_class = "due-date";

    /**
     * @brief Sets up drag and drop functionality for the card widget.
     */
    void setup_drag_and_drop();

    /**
     * @brief Opens the color dialog for selecting card cover color.
     */
    void open_color_dialog();

    /**
     * @brief Opens the card details dialog.
     */
    void open_card_details_dialog();

    /**
     * @brief Enables renaming of the card.
     */
    void on_rename();

    /**
     * @brief Disables renaming of the card.
     */
    void off_rename();

    /**
     * @brief Confirms changes made to the card.
     */
    void on_confirm_changes();

    /**
     * @brief Cancels changes made to the card.
     */
    void on_cancel_changes();

    /**
     * @brief Sets the card's color
     */
    void _set_color(const Gdk::RGBA& color);

    /**
     * @brief Clears the card cover color.
     */
    void clear_color();

    void cleanup() override;
};

}  // namespace ui