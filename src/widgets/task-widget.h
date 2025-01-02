#include <core/task.h>
#include <dialog/card-dialog.h>
#include <gtkmm.h>

#include <memory>

namespace ui {
/**
 * @brief Class implementing controlling facilities of Task widget.
 *
 * The TaskWidget class is responsible for displaying and managing a single task
 * within the UI. It provides functionalities such as renaming, removing, and
 * converting tasks, as well as handling drag-and-drop operations. This widget
 * is typically used within a CardDetailsDialog.
 */
class TaskWidget : public Gtk::Box {
public:
    /**
     * @brief TaskWidget constructor.
     *
     * Initializes a new TaskWidget.
     *
     * @param card_widget Reference to the CardWidget this widget is related to.
     * @param task Smart pointer to the Task object associated with this widget.
     * @param is_new Boolean flag indicating if this is a new task.
     */
    TaskWidget(CardDetailsDialog& card_details_dialog, CardWidget& card_widget,
               std::shared_ptr<Task> task, bool is_new = false);

    /**
     * @brief Destructor.
     */
    ~TaskWidget() override;

    /**
     * @brief Returns the Task object smart pointer associated with this widget.
     *
     * @return std::shared_ptr<Task> Smart pointer to the Task object.
     */
    std::shared_ptr<Task> get_task();

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

    Gtk::Label task_label;              ///< Label displaying the task name.
    Gtk::Revealer task_entry_revealer;  ///< Revealer for the task entry.
    Gtk::Entry task_entry;              ///< Entry for editing the task name.
    Gtk::CheckButton
        task_checkbutton;  ///< Checkbox for marking the task as completed.

    const Glib::RefPtr<Gio::Menu>
        menu_model;  ///< Menu model for the task widget.
    const Glib::RefPtr<Gio::SimpleActionGroup>
        group;  ///< Action group for the task widget.
    Gtk::PopoverMenu
        popover_menu;  ///< Popover menu for additional task actions.

    std::shared_ptr<Task>
        task;  ///< Smart pointer to the associated Task object.
    CardDetailsDialog&
        card_details_dialog;  ///< Reference to the parent CardDetailsDialog.

    bool is_new;  ///< Flag indicating if this is a new task.
};
}  // namespace ui
