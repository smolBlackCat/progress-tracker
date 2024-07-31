#include <gtkmm.h>

#include <memory>

#include "../core/task.h"
#include "../dialog/card-dialog.h"

namespace ui {
class TaskWidget : public Gtk::Box {
public:
    TaskWidget(BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder,
               CardDetailsDialog& card_details_dialog,
               std::shared_ptr<Task> task);

protected:
    void on_rename();
    void on_remove();

    Gtk::Label* task_label;
    Gtk::Revealer* task_entry_revealer;
    Gtk::Entry* task_entry;
    Gtk::CheckButton* task_checkbutton;
    Gtk::Popover popover;

    std::shared_ptr<Task> task;
    CardDetailsDialog& card_details_dialog;
};
}  // namespace ui
