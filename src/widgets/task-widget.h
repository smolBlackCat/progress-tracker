#include <core/task.h>
#include <dialog/card-dialog.h>
#include <gtkmm.h>

#include <memory>

namespace ui {
class TaskWidget : public Gtk::Box {
public:
    TaskWidget(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               CardDetailsDialog& card_details_dialog,
               std::shared_ptr<Task> task);

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

