#include "task-widget.h"

namespace ui {

TaskWidget::TaskWidget(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       CardDetailsDialog& card_details_dialog, Task& task)
    : Gtk::Box{cobject}, card_details_dialog{card_details_dialog}, task{task} {
    task_label = builder->get_widget<Gtk::Label>("task-label");
    task_entry_revealer =
        builder->get_widget<Gtk::Revealer>("task-entry-revealer");
    task_entry = builder->get_widget<Gtk::Entry>("task-entry");
    task_checkbutton =
        builder->get_widget<Gtk::CheckButton>("task-checkbutton");
}

TaskWidget* TaskWidget::create(CardDetailsDialog& card_details_dialog,
                               Task& task) {
    auto builder = Gtk::Builder::create_from_resource(
        "/io/github/smolblackcat/Progress/checklist-item-widget.ui");

    return Gtk::Builder::get_widget_derived<TaskWidget>(
        builder, "root", card_details_dialog, task);
}

void TaskWidget::on_rename() {}

void TaskWidget::on_remove() {}
}  // namespace ui
