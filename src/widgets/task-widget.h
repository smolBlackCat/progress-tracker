#pragma once

#include <core/task.h>
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
     * @param task Task data
     * @param editing_mode bool value indicating whether to start this widget in
     * editing mode
     */
    TaskWidget(CardDetailsDialog& card_details_dialog,
               const std::shared_ptr<Task>& task, bool editing_mode = false);

    /**
     * @brief Returns the Task instance pointer
     *
     * @return Task instance pointer.
     */
    std::shared_ptr<Task> task() const;

protected:
    /**
     * @brief Handles the task done signal event
     */
    void done_handler(bool done);

    /**
     * @brief Handles the rename action.
     */
    void on_rename();

    /**
     * @brief Handles the end of the rename action.
     */
    void off_rename();

    /**
     * @brief Handles the remove action.
     */
    void on_remove();

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

    Gtk::Label m_label;
    Gtk::Revealer m_entry_revealer;
    Gtk::Entry m_entry;
    Gtk::CheckButton m_checkbutton;
    Gtk::PopoverMenu m_popover;
    const Glib::RefPtr<Gtk::EventControllerFocus> focus_controller;

    CardDetailsDialog& m_card_dialog;

    const Glib::RefPtr<Gio::Menu> m_menu_model;
    const Glib::RefPtr<Gio::SimpleActionGroup> m_action_group;

    std::shared_ptr<Task> m_task;
    bool is_new;
};

}  // namespace ui

