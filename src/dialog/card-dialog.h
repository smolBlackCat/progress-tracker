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
 * user all details belonging to a given card, like its name, extra tasks and
 * general notes about the card.
 */
class CardDetailsDialog {
public:
    static CardDetailsDialog* create(CardWidget& card_widget);

    /**
     * @brief Default CardDetailsDialog constructor
     */
    CardDetailsDialog();

    ~CardDetailsDialog();

    /**
     * @brief Removes a TaskWidget from the checklist area of the dialog
     *
     * @param task TaskWidget object reference to be removed
     */
    void remove_task(TaskWidget& task);

    void reorder_task_widget(TaskWidget& next, TaskWidget& sibling);

    /**
     * @brief Opens the details of a card widget
     *
     * @param parent transient window
     * @param card_widget card widget pointer to modify settings from
     */
    void open(Gtk::Window& parent, CardWidget* card_widget);

    void on_save();

    void close();

    void update_due_date_label();

    /**
     * @brief Returns the CardWidget object pointer
     */
    CardWidget* get_card_widget();

protected:
    void on_add_task();
    void on_delete_card();
    void on_unset_due_date();
    void on_set_due_date();
    void clear();

    Glib::RefPtr<Gtk::Builder> builder;

    Glib::RefPtr<Glib::Object> adw_dialog;
    Gtk::Entry* card_title_entry;
    Gtk::Button checklist_add_button;
    Gtk::Button *unset_due_date_button, *card_delete_button;
    Gtk::MenuButton* date_menubutton;
    Gtk::Calendar* calendar;
    Gtk::Revealer* checkbutton_revealer;
    Gtk::CheckButton* checkbutton;
    Gtk::Box* tasks_box;
    Glib::RefPtr<Gtk::TextBuffer> notes_textbuffer;

    CardWidget* cur_card_widget;
    std::vector<TaskWidget*> tasks_tracker;

private:
    void _add_task(const std::shared_ptr<Task> task, bool is_new = false);
};
}  // namespace ui
