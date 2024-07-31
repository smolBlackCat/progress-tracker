#include "task-widget.h"

namespace ui {

TaskWidget::TaskWidget(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       CardDetailsDialog& card_details_dialog,
                       std::shared_ptr<Task> task)
    : Gtk::Box{cobject}, card_details_dialog{card_details_dialog}, task{task} {
    task_label = builder->get_widget<Gtk::Label>("task-label");
    task_entry_revealer =
        builder->get_widget<Gtk::Revealer>("task-entry-revealer");
    task_entry = builder->get_widget<Gtk::Entry>("task-entry");
    task_checkbutton =
        builder->get_widget<Gtk::CheckButton>("task-checkbutton");
}

void TaskWidget::on_rename() {}

void TaskWidget::on_remove() {}
}  // namespace ui
