#pragma once

#include <dialog/card-dialog.h>
#include <gtkmm.h>

#include "base-item.h"
#include "glibmm/extraclassinit.h"

extern "C" {
static void task_class_init(void* klass, void* user_data);
static void task_init(GTypeInstance* instance, void* klass);
}

namespace ui {

class TaskInit : public Glib::ExtraClassInit {
public:
    TaskInit();
};

/**
 * @brief TaskWidget class
 */
class TaskWidget : public TaskInit, public BaseItem {
public:
    /**
     * @brief TaskWidget constructor.
     *
     * @param card_details_dialog CardDetailsDialog reference to hold this
     * widget
     *
     * @param name task's title
     * 
     * @param complete task's complete status
     */
    TaskWidget(CardDialog& card_details_dialog, const std::string& title,
               bool complete = false);

    void set_title(const std::string& title);
    void set_complete(bool complete = true);

    std::string get_title() const;
    bool get_complete() const;

    sigc::signal<void(std::string, std::string)>& signal_name_changed();
    sigc::signal<void()>& signal_complete_changed();

protected:
    /**
     * @brief Handles the rename action.
     */
    void on_rename();

    /**
     * @brief Handles the end of the rename action.
     */
    void off_rename();

    /**
     * @brief Checkbox toggled event handler
     */
    void on_checkbox();

    /**
     * @brief Converts the task to a different type.
     *
     * @param parent CardWidget instance reference.
     */
    void on_convert();

    /**
     * @brief Sets up drag-and-drop functionality for the task widget.
     */
    void setup_drag_and_drop();

    void cleanup() override;

    // Widgets
    Gtk::Label m_label;
    Gtk::Revealer m_entry_revealer;
    Gtk::Entry m_entry;
    Gtk::CheckButton m_checkbutton;
    Gtk::PopoverMenu m_popover;

    CardDialog& m_card_dialog;
    CardWidget* m_card_widget;

    std::string m_title;

    const Glib::RefPtr<Gtk::EventControllerFocus> m_focus_controller;
    const Glib::RefPtr<Gio::Menu> m_menu_model;
    const Glib::RefPtr<Gio::SimpleActionGroup> m_action_group;

    sigc::signal<void(std::string, std::string)> m_name_changed_signal;
    sigc::signal<void()> m_complete_changed_signal;
};

}  // namespace ui
