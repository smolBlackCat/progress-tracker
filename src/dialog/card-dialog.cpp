#include "card-dialog.h"
#include <utility>

#include "../widgets/card.h"
#include "../widgets/task-widget.h"
#include "core/task.h"

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
    Glib::RefPtr<Gtk::TextBuffer> notes_textbuffer =
        builder->get_object<Gtk::TextBuffer>("notes-textbuffer");

    checklist_add_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_add_button_click));
    checklist_togglebutton->signal_toggled().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_toggle));
}

void CardDetailsDialog::add_task() {}

void CardDetailsDialog::remove_task(TaskWidget& task) {}

void CardDetailsDialog::set_card_widget(CardWidget* card_widget) {
    task_name_entry->set_text(card_widget->get_card()->get_name());
}

CardWidget* CardDetailsDialog::get_card_widget() { return nullptr; }

CardDetailsDialog* CardDetailsDialog::create() {
    auto builder = Gtk::Builder::create_from_resource(
        "/io/github/smolblackcat/Progress/card-details-dialog.ui");

    return builder->get_widget_derived<ui::CardDetailsDialog>(builder,
                                                              "card-dialog");
}

void CardDetailsDialog::on_add_button_click() {
    checklist_box->append(*Gtk::make_managed<Gtk::Button>("This is a Test"));
}

void CardDetailsDialog::on_toggle() {
    checklist_revealer->set_reveal_child(checklist_togglebutton->get_active());
}

void CardDetailsDialog::save() {}
}  // namespace ui
