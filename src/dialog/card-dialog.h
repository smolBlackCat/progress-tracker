#pragma once
#include <adwaita.h>
#include <core/task.h>
#include <gtkmm.h>

#include <memory>

namespace ui {

class TaskWidget;   
class CardWidget;

/**
 * @brief Class implementing a Dialog presenting the card details.
 *
 * @details The CardDetailsDialog (AdwDialog) main task is to present the
 * user all details belonging to a given card, like its name, extra tasks
 * and general notes about the card.
 */
class CardDetailsDialog {
public:
    /**
     * @brief Creates a new CardDetailsDialog instance.
     *
     * @param card_widget Reference to the CardWidget associated with this
     * dialog.
     * @return Pointer to the created CardDetailsDialog instance.
     */
    static CardDetailsDialog* create(CardWidget& card_widget);

    /**
     * @brief Default CardDetailsDialog constructor.
     */
    CardDetailsDialog();

    /**
     * @brief Destructor.
     */
    ~CardDetailsDialog();

    TaskWidget* add_task(const Task& task);

    TaskWidget* insert_new_task_after(const Task& task, TaskWidget* sibling);

    /**
     * @brief Removes a TaskWidget from the checklist area of the dialog.
     *
     * @param task TaskWidget object reference to be removed.
     */
    void remove_task(TaskWidget& task_widget);

    /**
     * @brief Reorders task widgets within the checklist area.
     *
     * @param next Reference to the TaskWidget to be placed after the
     * sibling.
     * @param sibling Reference to the TaskWidget to be placed before the
     * next.
     */
    void reorder_task_widget(TaskWidget& next, TaskWidget& sibling);

    /**
     * @brief Opens the details of a card widget.
     *
     * @param parent Transient window.
     * @param card_widget Pointer to the CardWidget to modify settings from.
     */
    void open(Gtk::Window& parent, CardWidget* card_widget);

    /**
     * @brief Saves the changes made to the card details.
     */
    void on_save();

    /**
     * @brief Closes the card details dialog.
     */
    void close();

    /**
     * @brief Updates the due date label in the dialog.
     */
    void update_due_date_label();

    /**
     * @brief Returns the CardWidget object pointer.
     *
     * @return Pointer to the CardWidget object.
     */
    CardWidget* get_card_widget();

protected:
    /**
     * @brief Handles the action to add a new task.
     */
    void on_add_task();

    /**
     * @brief Handles the action to delete the card.
     */
    void on_delete_card();

    /**
     * @brief Handles the action to unset the due date.
     */
    void on_unset_due_date();

    /**
     * @brief Handles the action to set the due date.
     */
    void on_set_due_date();

    /**
     * @brief Clears the dialog fields.
     */
    void clear();

    Glib::RefPtr<Gtk::Builder> builder;

    Glib::RefPtr<Glib::Object> m_adw_dialog;
    Gtk::Entry* m_title_entry;
    Gtk::Button m_checklist_add_button;
    Gtk::Button *m_unset_due_date_button, *m_card_delete_button;
    Gtk::MenuButton* m_date_menubutton;
    Gtk::Calendar* m_calendar;
    Gtk::Revealer* m_checkbutton_revealer;
    Gtk::CheckButton* m_checkbutton;
    Gtk::Box* m_tasks_box;
    Glib::RefPtr<Gtk::TextBuffer> m_notes_textbuffer;

    CardWidget* m_card_widget;
    std::vector<TaskWidget*> m_tasks_tracker;

private:
    /**
     * @brief Adds a new task to the checklist.
     *
     * @param task Shared pointer to the Task object.
     * @param is_new Boolean flag indicating if this is a new task.
     *
     * @return Pointer to the newly added TaskWidget.
     */
    TaskWidget* _add_task(const std::shared_ptr<Task>& task,
                          bool is_new = false);
};

}  // namespace ui
