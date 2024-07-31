#include "task-widget.h"

#include "gtkmm/eventcontrollerkey.h"

namespace ui {

TaskWidget::TaskWidget(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       CardDetailsDialog& card_details_dialog,
                       std::shared_ptr<Task> task)
    : Gtk::Box{cobject}, card_details_dialog{card_details_dialog}, task{task} {
    task_label = builder->get_widget<Gtk::Label>("task-label");
    task_label->set_label(task->get_name());
    task_entry_revealer =
        builder->get_widget<Gtk::Revealer>("task-entry-revealer");
    task_entry = builder->get_widget<Gtk::Entry>("task-entry");
    task_checkbutton =
        builder->get_widget<Gtk::CheckButton>("task-checkbutton");

    task_checkbutton->set_active(task->get_done());
    task_checkbutton->signal_toggled().connect(
        [this]() { this->task->set_done(task_checkbutton->get_active()); });

    auto gesture_click = Gtk::GestureClick::create();
    gesture_click->signal_released().connect(
        [this](int n_pressed, double x, double y) {
            if (n_pressed >= 2) {
                on_rename();
            }
        });
    this->add_controller(gesture_click);

    auto focus_controller = Gtk::EventControllerFocus::create();
    focus_controller->signal_leave().connect(
        sigc::mem_fun(*this, &TaskWidget::off_rename));
    task_entry->add_controller(focus_controller);

    auto key_controller = Gtk::EventControllerKey::create();
    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (task_entry_revealer->get_child_revealed()) {
                switch (keyval) {
                    case (GDK_KEY_Return): {
                        off_rename();
                        break;
                    }
                }
            }
        },
        false);
    task_entry->add_controller(key_controller);
}

void TaskWidget::on_rename() {
    task_entry_revealer->set_reveal_child();
    task_label->set_visible(false);

    task_entry->set_text(task->get_name());
}

void TaskWidget::off_rename() {
    task_entry_revealer->set_reveal_child(false);
    task_label->set_visible(true);

    std::string new_label = task_entry->get_text();
    if (new_label != task->get_name()) {
        task_label->set_text(new_label);
        task->set_name(new_label);
    }
}

void TaskWidget::on_remove() {}
}  // namespace ui
