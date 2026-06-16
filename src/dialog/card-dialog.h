#pragma once
#include <adwaita.h>
#include <core/task.h>
#include <gtkmm.h>

#include <chrono>
#include <utility>

namespace ui {

class TaskWidget;
class CardWidget;
/**
 * @brief Card Dialog
 */
class CardDialog {
public:
    /**
     * @brief Default CardDetailsDialog constructor.
     */
    CardDialog();

    /**
     * @brief Destructor.
     */
    ~CardDialog();

    void set_title(const std::string& title);
    void set_notes(const std::string& notes);
    void set_deadline(const Glib::Date& deadline = {});
    void set_complete(bool complete = true);

    void append(TaskWidget& task);
    void insert_after(TaskWidget& next, TaskWidget& sibling);
    void remove_task(TaskWidget& task_widget);

    /**
     * @brief Reorders task widgets within the checklist area.
     *
     * @param next Reference to the TaskWidget to be placed after the
     * sibling.
     * @param sibling Reference to the TaskWidget to be placed before the
     * next.
     */
    void reorder(TaskWidget& next, TaskWidget& sibling);

    /**
     * @brief Opens the details of a card widget.
     *
     * @param parent Transient window.
     * @param card_widget Pointer to the CardWidget to modify settings from.
     */
    void open(Gtk::Window& parent, CardWidget* card_widget);

    void close();

    std::string get_title() const;
    std::string get_notes() const;
    std::chrono::year_month_day get_deadline() const;
    bool get_complete() const;
    std::pair<int, int> get_completion_ratio() const;

    /**
     * @brief Returns the CardWidget object pointer.
     *
     * @return Pointer to the CardWidget object.
     */
    CardWidget* card_widget();

    sigc::signal<void(std::string, std::string)>& signal_name_changed();
    sigc::signal<void(std::string, std::string)>& signal_notes_changed();
    sigc::signal<void(Glib::Date, Glib::Date)>& signal_deadline_changed();
    sigc::signal<void()>& signal_complete_changed();
    sigc::signal<void(TaskWidget*, int)>& signal_task_added();
    sigc::signal<void(TaskWidget*)>& signal_task_removed();
    sigc::signal<void(TaskWidget*, TaskWidget*, bool)>& signal_task_reordered();
    sigc::signal<void(CardWidget*)>& signal_open();
    sigc::signal<void()>& signal_closed();

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

    // Widgets
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

    // Signals
    sigc::signal<void(std::string, std::string)> m_name_changed_signal;
    sigc::signal<void(std::string, std::string)> m_notes_changed_signal;
    sigc::signal<void(Glib::Date, Glib::Date)> m_deadline_changed_signal;
    sigc::signal<void()> m_complete_changed_signal;
    sigc::signal<void(TaskWidget*, int)> m_task_add_signal;
    sigc::signal<void(TaskWidget*)> m_task_remove_signal;
    sigc::signal<void(TaskWidget*, TaskWidget*, bool up)> m_task_reorder_signal;
    sigc::signal<void(CardWidget*)> m_card_dialog_opened_signal;
    sigc::signal<void()> m_card_dialog_closed_signal;

    CardWidget* m_card_widget;

    std::chrono::year_month_day m_deadline;
    int m_n_tasks_complete = 0;
    int m_n_tasks = 0;
};

}  // namespace ui
