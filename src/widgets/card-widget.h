#pragma once

#include <core/card.h>
#include <gtkmm.h>

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

    const static std::unordered_map<CoverColor, Gdk::RGBA> CARD_COLORS;

    /**
     * @brief Helper method for creating a tooltip text for the card widget.
     *
     * @return the tooltip text string.
     */
    static std::string card_widget_tooltip_text(int n_task = 0,
                                                int n_tasks_complete = 0,
                                                Glib::Date deadline = {},
                                                bool complete = false,
                                                const std::string& notes = {});

    /**
     * @brief Constructs a CardWidget object.
     *
     * @param title card's title
     *
     * @param cover_color card's cover color
     *
     * @param deadline card's deadline. When deadline is a valid date
     * representation, the set complete value is valid.
     *
     * @param complete flags whether the card is marked as complete. This flag
     * is only considered if a deadline is set, othewise it will be ignored
     *
     * @param n_tasks number of tasks related to this card
     *
     * @param n_tasks_complete number of tasks complete. Cannot be greater than
     * the amount of tasks
     */
    CardWidget(const std::string& title, Gdk::RGBA cover_color = {},
               Glib::Date deadline = {}, bool complete = false, int n_tasks = 0,
               int n_tasks_complete = 0);

    /**
     * @brief Sets the title of the card.
     *
     * @param label new title for the card.
     */
    void set_title(const std::string& title);

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
    void set_cover_color(Gdk::RGBA color);

    /**
     * @brief Sets the card deadline
     *
     * @param new_date card's deadline
     * @param complete whether the card should be marked as complete
     */
    void set_deadline_label(const Glib::Date& new_date = {},
                            bool complete = false);

    /**
     * @param Sets the completion label visible and sets the ratio of
     * complete/tasks as a label
     */
    void set_completion_label(int n_tasks, int n_tasks_complete);

    /**
     * @param Marks this card as complete. This method only has effect if there
     * is a deadline set
     */
    void set_complete(bool complete = true);

    /**
     * @brief Updates the deadline label if and only if a deadline is set
     */
    void update_deadline_label();

    /**
     * @brief Removes itself from the associated CardlistWidget object.
     */
    void remove_from_parent();

    std::string get_title() const;

    Gdk::RGBA get_cover_color() const;

    bool get_complete() const;
    bool is_deadline_set() const;

    CardlistWidget* parent() const;

    sigc::signal<void(std::string, std::string)>& signal_name_changed();
    sigc::signal<void(Gdk::RGBA, Gdk::RGBA)>& signal_color_changed();
    sigc::signal<void(CardWidget*, CardlistWidget*)>& signal_card_received();
    sigc::signal<void()>& signal_card_dialog_opened();
    sigc::signal<void()>& signal_card_dialog_closed();

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

    // Widgets
    Gtk::Box m_root;
    Gtk::Revealer m_card_cover_revealer, m_card_entry_revealer;
    Gtk::Picture m_card_cover_picture;
    Gtk::Label m_card_label, m_completion_label, m_deadline_label;
    Gtk::Entry m_card_entry;
    Gtk::MenuButton m_card_menu_button;
    CardPopover m_fixed_card_popover, m_mouse_card_popover;

    // Signals
    sigc::signal<void(std::string, std::string)> m_name_changed_signal;
    sigc::signal<void(Gdk::RGBA, Gdk::RGBA)> m_color_changed_signal;

    // incoming_card, sibling, incoming_parent
    sigc::signal<void(CardWidget*, CardlistWidget*)> m_card_received_signal;

    // Controllers
    Glib::RefPtr<Gtk::EventControllerFocus> m_focus_ctrl;
    Glib::RefPtr<Gtk::EventControllerKey> m_key_ctrl;
    Glib::RefPtr<Gtk::GestureClick> m_click_ctrl;

    Glib::Date m_date;
    Gdk::RGBA m_color;
    bool m_complete;
    ui::CardlistWidget* m_parent;

    void setup_drag_and_drop();
    void open_card_dialog();

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
    void update_completion_label(int n_tasks, int n_tasks_complete);

    void cleanup() override;

private:
    void setup_widgets();
    void __set_cover_color(const Gdk::RGBA& color);
    void __clear_cover_color();
};
}  // namespace ui