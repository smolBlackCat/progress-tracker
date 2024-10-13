#include "card-dialog.h"

#include <glibmm/i18n.h>
#include <widgets/card.h>
#include <widgets/task-widget.h>

#include "glibmm/ustring.h"

namespace ui {

CardDetailsDialog::CardDetailsDialog(CardWidget& card_widget)
    : card_widget{card_widget},
      checklist_add_button{_("Add Task")},
      builder{Gtk::Builder::create_from_resource(
          "/io/github/smolblackcat/Progress/card-details-dialog.ui")},
      card_title_entry{builder->get_widget<Gtk::Entry>("card-title-entry")},
      unset_due_date_button{
          builder->get_widget<Gtk::Button>("unset-due-date-button")},
      card_delete_button{
          builder->get_widget<Gtk::Button>("delete-card-button")},
      date_menubutton{builder->get_widget<Gtk::MenuButton>("date-menubutton")},
      calendar{builder->get_widget<Gtk::Calendar>("calendar")},
      checkbutton_revealer{
          builder->get_widget<Gtk::Revealer>("checkbutton-revealer")},
      checkbutton{builder->get_widget<Gtk::CheckButton>("checkbutton")},
      tasks_box{builder->get_widget<Gtk::Box>("tasks-box")},
      notes_textbuffer{
          builder->get_object<Gtk::TextBuffer>("notes-textbuffer")},
      dialog{builder->get_object("card-dialog")} {
    checklist_add_button.signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_add_task));

    auto card_ptr = this->card_widget.get_card();

    card_delete_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_delete_card));

    if (card_ptr->get_due_date().ok()) {
        auto sys_days = std::chrono::sys_days(card_ptr->get_due_date());
        sys_days++;
        std::time_t time = std::chrono::system_clock::to_time_t(sys_days);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%F");
        date_menubutton->set_label(ss.str());
        checkbutton_revealer->set_reveal_child(true);
        checkbutton->set_active(card_ptr->get_complete());
    }

    checkbutton->signal_toggled().connect([this]() {
        this->card_widget.get_card()->set_complete(
            this->checkbutton->get_active());
        this->card_widget.update_due_date_label_style();
    });

    calendar->signal_day_selected().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_set_due_date));

    unset_due_date_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_unset_due_date));

    g_signal_connect(dialog->gobj(), "close-attempt",
                     G_CALLBACK(+[](AdwDialog* self, gpointer data) {
                         reinterpret_cast<CardDetailsDialog*>(data)->on_save();
                         reinterpret_cast<CardDetailsDialog*>(data)->close();
                     }),
                     this);

    tasks_box->append(checklist_add_button);

    card_title_entry->set_text(card_ptr->get_name());
    for (auto& task : card_ptr->get_tasks()) {
        _add_task(task);
    }
    notes_textbuffer->set_text(card_ptr->get_notes());
}

CardDetailsDialog* CardDetailsDialog::create(CardWidget& card_widget) {
    auto card_dialog = new CardDetailsDialog{card_widget};
    return card_dialog;
}

CardDetailsDialog::~CardDetailsDialog() {}

void CardDetailsDialog::remove_task(TaskWidget& task) {
    auto card = card_widget.get_card();
    card->remove_task(task.get_task());
    tasks_box->remove(task);
    card_widget.update_complete_tasks();
}

void CardDetailsDialog::reorder_task_widget(TaskWidget& next,
                                            TaskWidget& sibling) {
    tasks_box->reorder_child_after(next, sibling);
    card_widget.get_card()->reorder_task(*next.get_task(), *sibling.get_task());
}

void CardDetailsDialog::open(Gtk::Window& parent) {
    adw_dialog_present(ADW_DIALOG(dialog->gobj()),
                       static_cast<Gtk::Widget&>(parent).gobj());
}

void CardDetailsDialog::close() {
    adw_dialog_force_close(ADW_DIALOG(dialog->gobj()));
    delete this;  // Suicide
}

CardWidget& CardDetailsDialog::get_card_widget() { return card_widget; }

void CardDetailsDialog::on_add_task() {
    _add_task(card_widget.get_card()->add_task(Task{_("New Task")}), true);
    card_widget.update_complete_tasks();
}

void CardDetailsDialog::on_save() {
    auto card = card_widget.get_card();
    std::string new_card_name = card_title_entry->get_text();
    std::string new_notes = notes_textbuffer->get_text();

    if (card->get_name() != new_card_name) {
        card_widget.set_label(new_card_name);
        card->set_name(new_card_name);
    }

    if (card->get_notes() != new_notes) {
        card->set_notes(new_notes);
    }

    card_widget.set_tooltip_text(card_widget.create_details_text());
}

void CardDetailsDialog::on_delete_card() {
    card_widget.remove_from_parent();
    close();
}

void CardDetailsDialog::on_unset_due_date() {
    auto card_ptr = card_widget.get_card();

    date_menubutton->set_label(_("Set Due Date"));
    card_ptr->set_due_date(Date{});
    checkbutton_revealer->set_reveal_child(false);
    card_widget.update_due_date();
}

void CardDetailsDialog::on_set_due_date() {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto card_ptr = card_widget.get_card();

    int y, m, d;
    calendar->get_date().get_ymd(y, m, d);
    auto new_date = Date{year{y}, month{static_cast<unsigned int>(m)},
                         day{static_cast<unsigned int>(d)}};
    card_ptr->set_due_date(new_date);
    auto sys_days = std::chrono::sys_days(new_date);
    sys_days++;
    std::time_t time = std::chrono::system_clock::to_time_t(sys_days);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%F");
    date_menubutton->set_label(ss.str());
    checkbutton_revealer->set_reveal_child(true);
    card_widget.update_due_date();
}

void CardDetailsDialog::_add_task(const std::shared_ptr<Task> task,
                                  bool is_new) {
    auto builder = Gtk::Builder::create_from_resource(
        "/io/github/smolblackcat/Progress/checklist-item-widget.ui");
    auto task_widget = Gtk::Builder::get_widget_derived<TaskWidget>(
        builder, "task-widget", *this, card_widget, task, is_new);
    tasks_box->append(*task_widget);
}
}  // namespace ui
