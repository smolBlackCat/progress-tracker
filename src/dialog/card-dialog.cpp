#include "card-dialog.h"

#include <glibmm/i18n.h>
#include <widgets/card.h>
#include <widgets/task-widget.h>

namespace ui {
CardDetailsDialog::CardDetailsDialog(BaseObjectType* cobject,
                                     const Glib::RefPtr<Gtk::Builder>& builder,
                                     CardWidget& card_widget)
    : Gtk::Dialog{cobject},
      card_widget{card_widget},
      task_name_entry{builder->get_widget<Gtk::Entry>("task-name-entry")},
      checklist_add_button{
          builder->get_widget<Gtk::Button>("checklist-add-button")},
      checklist_togglebutton{
          builder->get_widget<Gtk::ToggleButton>("checklist-togglebutton")},
      checklist_revealer{
          builder->get_widget<Gtk::Revealer>("checklist-revealer")},
      checklist_box{builder->get_widget<Gtk::Box>("checklist-box")},
      notes_textbuffer{
          builder->get_object<Gtk::TextBuffer>("notes-text-buffer")} {
    checklist_add_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_add_button_click));
    checklist_togglebutton->signal_toggled().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_toggle));

    signal_close_request().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::save), false);

    auto card_ptr = this->card_widget.get_card();

    task_name_entry->set_text(card_ptr->get_name());
    // Load tasks into checklist box
    for (auto& task : card_ptr->get_tasks()) {
        _add_task(task);
    }
    if (card_ptr->get_tasks().size() != 0) {
        checklist_togglebutton->set_active();
    }
    notes_textbuffer->set_text(card_ptr->get_notes());
}

CardDetailsDialog::~CardDetailsDialog() {}

void CardDetailsDialog::remove_task(TaskWidget& task) {
    auto card = card_widget.get_card();
    checklist_box->remove(task);
    card->remove_task(task.get_task());
    card_widget.update_completed();

    if (card->get_tasks().size() == 0) {
        card_widget.hide_progress_bar();
    }
}

CardWidget& CardDetailsDialog::get_card_widget() { return card_widget; }

CardDetailsDialog* CardDetailsDialog::create(CardWidget& card_widget) {
    auto builder = Gtk::Builder::create_from_resource(
        "/io/github/smolblackcat/Progress/card-details-dialog.ui");

    return Gtk::manage(Gtk::Builder::get_widget_derived<CardDetailsDialog>(
        builder, "card-dialog", card_widget));
}

void CardDetailsDialog::on_add_button_click() {
    _add_task(card_widget.get_card()->add_task(Task{_("New Task")}));
    checklist_togglebutton->set_active();
    card_widget.hide_progress_bar(false);
}

void CardDetailsDialog::on_toggle() {
    checklist_revealer->set_reveal_child(checklist_togglebutton->get_active());
}

bool CardDetailsDialog::save() {
    auto card = card_widget.get_card();

    card_widget.set_label(task_name_entry->get_text());
    card->set_name(task_name_entry->get_text());

    card->set_notes(notes_textbuffer->get_text());
    card_widget.set_tooltip_text(notes_textbuffer->get_text());

    return false;
}

void CardDetailsDialog::_add_task(const std::shared_ptr<Task> task) {
    auto builder = Gtk::Builder::create_from_resource(
        "/io/github/smolblackcat/Progress/checklist-item-widget.ui");
    auto task_widget = Gtk::manage(Gtk::Builder::get_widget_derived<TaskWidget>(
        builder, "task-widget", *this, task));
    checklist_box->append(*task_widget);
}
}  // namespace ui
