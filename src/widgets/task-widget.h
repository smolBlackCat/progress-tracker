#pragma once

#include <core/task.h>
#include <dialog/card-dialog.h>
#include <gtkmm.h>

#include <memory>

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
 * @brief Class implementing controlling facilities of Task widget.
 *
 * The TaskWidget class is responsible for displaying and managing a single task
 * within the UI. It provides functionalities such as renaming, removing, and
 * converting tasks, as well as handling drag-and-drop operations. This widget
 * is typically used within a CardDetailsDialog.
 */
class TaskWidget : public TaskInit, public BaseItem {
public:
    /**
     * @brief TaskWidget constructor.
     *
     * Initializes a new TaskWidget.
     *
     * @param card_details_dialog Reference to the CardDetailsDialog this widget
     * is related to.
     * @param card_widget Reference to the CardWidget this widget is related to.
     * @param task Smart pointer to the Task object associated with this widget.
     * @param is_new Boolean flag indicating if this is a new task.
     */
    TaskWidget(CardDetailsDialog& card_details_dialog, CardWidget& card_widget,
               const std::shared_ptr<Task>& task, bool is_new = false);

    /**
     * @brief Returns the Task object smart pointer associated with this widget.
     *
     * @return std::shared_ptr<Task> Smart pointer to the Task object.
     */
    std::shared_ptr<Task> get_task();

protected:
    /**
     * @brief Handles the done action.
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
     * @brief Handles the checkbox toggle action.
     */
    void on_checkbox();

    /**
     * @brief Converts the task to a different type.
     *
     * @param parent Reference to the parent CardWidget.
     */
    void on_convert(CardWidget& parent);

    /**
     * @brief Sets up drag-and-drop functionality for the task widget.
     */
    void setup_drag_and_drop();

    void cleanup() override;

    Gtk::Label task_label;
    Gtk::Revealer task_entry_revealer;
    Gtk::Entry task_entry;
    Gtk::CheckButton task_checkbutton;
    Gtk::PopoverMenu popover_menu;

    const Glib::RefPtr<Gio::Menu> menu_model;
    const Glib::RefPtr<Gio::SimpleActionGroup> group;

    std::shared_ptr<Task> task;
    CardDetailsDialog& card_details_dialog;

    bool is_new;
};

}  // namespace ui