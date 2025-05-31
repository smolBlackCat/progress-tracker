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
    const static std::array<std::string, 3> TASKS_LABEL_CSS_CLASSES;

    const static std::array<std::string, 3> DATE_LABEL_CSS_CLASSES;

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
    void set_cover_color(const Gdk::RGBA& color);

    /**
     * @brief Removes itself from the associated CardlistWidget object.
     */
    void remove_from_parent();

    /**
     * @brief Changes the due date label color depending on the card's
     * situation. Red if it is past due date, green if it is complete and plain
     * color if it is due but not yet complete.
     */
    void update_due_date_label();

    /**
     * @brief Updates the label informing the number of complete tasks for this
     * card.
     */
    void update_complete_tasks_label();

    /**
     * @brief Retrieves the title of the card.
     *
     * @return the card's title string.
     */
    std::string get_title() const;

    /**
     * @brief Helper method for creating a tooltip text for the card widget.
     *
     * @return the tooltip text string.
     */
    std::string create_details_text() const;

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

protected:
    /** @brief Implements the card's popover menu
     *
     * The current implemented options are:
     * - Rename card
     * - Card Details
     * - A colour selection radiobutton group
     * - Delete card
     */
    class CardPopover : public Gtk::Popover {
    public:
        const static std::map<const char*, std::pair<const char*, const char*>>
            CARD_COLORS;

        /**
         * @brief Updates all CardPopover instances color
         */
        static void mass_color_select(CardWidget* key_card_widget,
                                      Gdk::RGBA color);

        /**
         * @brief Constructor for CardPopover class.
         *
         * @param card The card widget to set the color for.
         */
        CardPopover(CardWidget* card);

        /**
         * @brief Marks one of the color radio buttons as selected. If a color
         * that does not belong to CARD_COLORS, this call is ignored.
         *
         * @param color The color to set.
         * @param trigger Whether to trigger the color change signal.
         */
        void set_selected_color(Gdk::RGBA color, bool trigger = true);

        /**
         * @brief Disable this card popover colour setting signals
         */
        void disable_color_signals();

        /**
         * @brief Enable this card popover colour setting signals
         */
        void enable_color_signals();

    protected:
        static std::map<CardWidget*, std::vector<CardPopover*>> card_popovers;

        /**
         * @brief Helper method for creating a color setting closure to set the
         * color of the card widget.
         *
         * @param card The card widget to set the color for.
         * @param color The color to set.
         * @return A closure that sets the color of the card widget.
         */
        std::function<void()> color_setting_thunk(CardWidget* card,
                                                  Gtk::CheckButton* checkbutton,
                                                  Gdk::RGBA color);
        CardWidget* card_widget;
        Gtk::Box root;
        std::map<const char*, Gtk::Button*> action_buttons;
        std::map<const char*,
                 std::tuple<Gtk::CheckButton*, const char*, sigc::connection>>
            color_buttons;
    };

    friend class CardPopover;

    bool is_new;
    Gtk::Box root_box;
    Gtk::Revealer card_cover_revealer, card_entry_revealer;
    Gtk::Picture card_cover_picture;
    Gtk::Label card_label, complete_tasks_label, due_date_label;
    Gtk::Entry card_entry;
    Gtk::MenuButton card_menu_button;
    CardPopover card_menu_popover, card_menu_popover2;

    Glib::RefPtr<Gtk::EventControllerFocus> focus_controller;
    Glib::RefPtr<Gtk::EventControllerKey> key_controller;
    Glib::RefPtr<Gtk::GestureClick> click_controller;
    Glib::RefPtr<Gtk::ColorDialog> color_dialog;

    ui::CardlistWidget* parent;
    std::shared_ptr<Card> card;

    /**
     * @brief Handles the task appending event. It updates the tasks complete
     * label
     *
     * @param task shared pointer to the task being appended.
     */
    void on_append_handle(std::shared_ptr<Task> task);

    /**
     * @brief Handles the task removal event
     *
     * @param task shared pointer to the task being removed.
     */
    void on_remove_handle(std::shared_ptr<Task> task);

    /**
     * @brief Updates the label informing the due date for this card.
     */
    void due_date_handler(Date old, Date new_);

    /**
     * @brief Handles the color change event.
     *
     * @param old_color old color.
     * @param new_color new color.
     */
    void color_handler(Color old, Color new_);

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
     * @brief Changes the complete tasks label color depending on the amount of
     * tasks complete. Green if all tasks are complete, Yellow if half of the
     * tasks are complete and Red if less than half of the tasks are complete.
     *
     * @param n_complete_tasks number of completed tasks.
     */
    void update_complete_tasks_style(unsigned long n_complete_tasks);

    /**
     * @brief Sets the card's color
     */
    void __set_cover_color(const Gdk::RGBA& color);

    /**
     * @brief Clears the card cover color.
     */
    void __clear_cover_color();

    void cleanup() override;
};

}  // namespace ui