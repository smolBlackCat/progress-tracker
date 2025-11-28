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
 * @brief Color options for a CardWidget cover
 */
enum class CoverColor { UNSET, BLUE, RED, ORANGE, GREEN, YELLOW, PURPLE };

/**
 * @brief Widget that represents a single card.
 */
class CardWidget : public CardInit, public BaseItem {
public:
    const static std::array<std::string, 3> TASKS_LABEL_CSS_CLASSES;

    const static std::array<std::string, 3> DATE_LABEL_CSS_CLASSES;

    const static std::map<CoverColor, Gdk::RGBA> CARD_COLORS;

    /**
     * @brief Constructs a CardWidget object.
     *
     * @param card a smart pointer pointing to a Card object.
     */
    CardWidget(std::shared_ptr<Card> card);

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
    void set_cover_color(CoverColor color);

    /**
     * @brief Sets the card deadline
     */
    void set_deadline(const Date& new_date);

    /**
     * @brief Removes itself from the associated CardlistWidget object.
     */
    void remove_from_parent();

    /**
     * @brief Changes the due date label color depending on the card's
     * situation. Red if it is past due date, green if it is complete and plain
     * color if it is due but not yet complete.
     */
    void update_deadline_label();

    /**
     * @brief Updates the label informing the number of complete tasks for this
     * card.
     */
    void update_completion_label();

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
    class CardPopover : public Gtk::PopoverMenu {
    public:
        /**
         * @brief Constructor for CardPopover class.
         *
         * @param card The card widget to set the color for.
         */
        CardPopover(CardWidget* card);

        ~CardPopover() override;

        /**
         * @brief Selects the radio button corresponding to the given color.
         *
         * If trigger is true, this method activates the color’s radio button
         * and emits the related color-change signal. It also updates all
         * sibling popovers associated with the same CardWidget by setting their
         * selected color without emitting additional signals.
         *
         * If trigger is false, the color’s radio button is activated silently
         * (the signal handler is temporarily blocked), and only this popover is
         * updated.
         *
         * @param color   The color to select.
         * @param trigger Whether to emit the color-change signal.
         */

        void set_selected_color(CoverColor color, bool trigger = true);

    protected:
        static std::map<CardWidget*, std::vector<CardPopover*>>
            registered_card_popovers;

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
                                                  CoverColor color);
        CardWidget* m_card_widget;
        std::map<CoverColor, std::tuple<Gtk::CheckButton*, sigc::connection>>
            m_color_radio_button_map;
    };

    friend class CardPopover;

    Gtk::Box m_root;
    Gtk::Revealer m_card_cover_revealer, m_card_entry_revealer;
    Gtk::Picture m_card_cover_picture;
    Gtk::Label m_card_label, m_completion_label, m_deadline_label;
    Gtk::Entry m_card_entry;
    Gtk::MenuButton m_card_menu_button;
    CardPopover m_fixed_card_popover, m_mouse_card_popover;

    Glib::RefPtr<Gtk::EventControllerFocus> m_focus_ctrl;
    Glib::RefPtr<Gtk::EventControllerKey> m_key_ctrl;
    Glib::RefPtr<Gtk::GestureClick> m_click_ctrl;

    ui::CardlistWidget* m_cardlist_widget;
    std::shared_ptr<Card> m_card;

    /**
     * @brief Sets up drag and drop functionality for the card widget.
     */
    void setup_drag_and_drop();

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
     * @brief Changes the complete tasks label color depending on the amount of
     * tasks complete. Green if all tasks are complete, Yellow if half of the
     * tasks are complete and Red if less than half of the tasks are complete.
     *
     * @param n_complete_tasks number of completed tasks.
     */
    void set_completion_label_color(unsigned long n_complete_tasks);

    void cleanup() override;

private:
    void setup_widgets();
    void __set_cover_color(const Gdk::RGBA& color);
    void __clear_cover_color();
};

}  // namespace ui