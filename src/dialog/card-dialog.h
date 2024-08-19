#pragma once
#include <core/task.h>
#include <gtkmm.h>

namespace ui {
class TaskWidget;
class CardWidget;

/**
 * @brief Class implementing a Dialog presenting the card details.
 *
 * @details The CardDetailsDialog Gtk::Dialog child main task is to show the
 * user all details belonging to a given card, like its name, extra tasks and
 * general notes about the card.
 */
class CardDetailsDialog : public Gtk::Dialog {
public:
    /**
     * @brief CardDetailsDialog constructor
     *
     * @details Despite being public, this constructor should not be used
     * directly, since the object is meant to be created using
     * Gtk::Builder::get_widget_derived(...) function
     *
     * @param cobject GtkDialog gobject pointer
     * @param builder Gtk::Builder object used to create this dialog window
     */
    CardDetailsDialog(BaseObjectType* cobject,
                      const Glib::RefPtr<Gtk::Builder>& builder,
                      CardWidget& card_widget);

    ~CardDetailsDialog() override;

    /**
     * @brief Removes a TaskWidget from the checklist area of the dialog
     *
     * @param task TaskWidget object reference to be removed
     */
    void remove_task(TaskWidget& task);

    /**
     * @brief Returns the CardWidget object pointer
     */
    CardWidget& get_card_widget();

    /**
     * @brief Creates an instance of this Dialog object. Ownership is given to
     * the caller
     *
     * @return CardDetailsDialog object pointer to the new instance
     */
    static CardDetailsDialog* create(CardWidget& card_widget);

protected:
    void on_add_button_click();
    void on_toggle();
    bool save();
    CardWidget& card_widget;

private:
    void _add_task(const std::shared_ptr<Task> task);

    Gtk::Entry* task_name_entry;
    Gtk::Button* checklist_add_button;
    Gtk::ToggleButton* checklist_togglebutton;
    Gtk::Revealer* checklist_revealer;
    Gtk::Box* checklist_box;
    Glib::RefPtr<Gtk::TextBuffer> notes_textbuffer;
};
}  // namespace ui
