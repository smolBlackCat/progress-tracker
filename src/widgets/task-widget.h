#include <core/task.h>
#include <dialog/card-dialog.h>
#include <gtkmm.h>

#include <memory>

namespace ui {
/**
 * @brief Class implementing controlling facilities of Task widget
 */
class TaskWidget : public Gtk::Box {
public:
    /**
     * @brief CardDetailsDialog constructor
     *
     * @details Despite being public, this constructor should not be used
     * directly, since the object is meant to be created using
     * Gtk::Builder::get_widget_derived(...) function
     *
     * @param cobject GtkBox gobject pointer
     * @param builder Gtk::Builder object used to create this widget
     * @param card_details_dialog CardDetailsDialog object reference from where
     * this widget belongs
     * @param task Task object smart pointer
     */
    TaskWidget(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               CardDetailsDialog& card_details_dialog,
               std::shared_ptr<Task> task);

    /**
     * @brief Returns the Task object smart pointer associated with this widget
     */
    std::shared_ptr<Task> get_task();

protected:
    void on_rename();
    void off_rename();
    void on_remove();

    Gtk::Label* task_label;
    Gtk::Revealer* task_entry_revealer;
    Gtk::Entry* task_entry;
    Gtk::CheckButton* task_checkbutton;

    const Glib::RefPtr<Gio::Menu> menu_model;
    const Glib::RefPtr<Gio::SimpleActionGroup> group;
    Gtk::PopoverMenu popover_menu;

    std::shared_ptr<Task> task;
    CardDetailsDialog& card_details_dialog;
};
}  // namespace ui
