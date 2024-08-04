#include "card-dialog.h"

#include <widgets/card.h>
#include <widgets/task-widget.h>

namespace ui {
CardDetailsDialog::CardDetailsDialog(BaseObjectType* cobject,
                                     const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Dialog{cobject} {
    task_name_entry = builder->get_widget<Gtk::Entry>("task-name-entry");
    checklist_add_button =
        builder->get_widget<Gtk::Button>("checklist-add-button");
    checklist_togglebutton =
        builder->get_widget<Gtk::ToggleButton>("checklist-togglebutton");
    checklist_revealer =
        builder->get_widget<Gtk::Revealer>("checklist-revealer");
    checklist_box = builder->get_widget<Gtk::Box>("checklist-box");
    notes_textbuffer =
        builder->get_object<Gtk::TextBuffer>("notes-text-buffer");

    checklist_add_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_add_button_click));
    checklist_togglebutton->signal_toggled().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_toggle));

    signal_close_request().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::save), false);
}

void CardDetailsDialog::remove_task(TaskWidget& task) {
    auto card = card_widget->get_card();
    checklist_box->remove(task);
    card->remove_task(task.get_task());
    card_widget->update_completed();

    if (card->get_tasks().size() == 0) {
        card_widget->hide_progress_bar();
    }
}

void CardDetailsDialog::set_card_widget(CardWidget* card_widget) {
    if (card_widget) {
        this->card_widget = card_widget;
        auto card_ptr = card_widget->get_card();

        task_name_entry->set_text(card_ptr->get_name());
        // Load tasks into checklist box
        for (auto& task : card_ptr->get_tasks()) {
            auto builder = Gtk::Builder::create_from_resource(
                "/io/github/smolblackcat/Progress/checklist-item-widget.ui");
            auto new_taskwidget = Gtk::Builder::get_widget_derived<TaskWidget>(
                builder, "task-widget", *this, task);

            checklist_box->append(*new_taskwidget);
        }
        notes_textbuffer->set_text(card_ptr->get_notes());
    }
}

CardWidget* CardDetailsDialog::get_card_widget() { return card_widget; }

CardDetailsDialog* CardDetailsDialog::create() {
    auto builder = Gtk::Builder::create_from_resource(
        "/io/github/smolblackcat/Progress/card-details-dialog.ui");

    return builder->get_widget_derived<CardDetailsDialog>(builder,
                                                          "card-dialog");
}

void CardDetailsDialog::on_add_button_click() {
    auto builder = Gtk::Builder::create_from_resource(
        "/io/github/smolblackcat/Progress/checklist-item-widget.ui");
    auto new_taskwidget = Gtk::Builder::get_widget_derived<TaskWidget>(
        builder, "task-widget", *this,
        card_widget->get_card()->add_task(Task{"New Task"}));

    checklist_box->append(*new_taskwidget);
    card_widget->hide_progress_bar(false);
}

void CardDetailsDialog::on_toggle() {
    checklist_revealer->set_reveal_child(checklist_togglebutton->get_active());
}

bool CardDetailsDialog::save() {
    auto card = card_widget->get_card();

    card_widget->set_label(task_name_entry->get_text());
    card->set_name(task_name_entry->get_text());

    card->set_notes(notes_textbuffer->get_text());
    card_widget->set_tooltip_text(notes_textbuffer->get_text());

    return false;
}
}  // namespace ui
