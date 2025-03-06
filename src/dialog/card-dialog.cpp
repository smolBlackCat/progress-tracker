#include "card-dialog.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <widgets/card-widget.h>
#include <widgets/task-widget.h>

#include <ctime>

#include "glibmm/ustring.h"

namespace ui {

CardDetailsDialog::CardDetailsDialog()
    : checklist_add_button{_("Add Task")},
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
      adw_dialog{builder->get_object("card-dialog")} {
    checklist_add_button.signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_add_task));

    card_delete_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_delete_card));

    checkbutton->signal_toggled().connect([this]() {
        this->cur_card_widget->get_card()->set_complete(
            this->checkbutton->get_active());
        this->cur_card_widget->update_due_date_label_style();
    });

    calendar->signal_day_selected().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_set_due_date));

    unset_due_date_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_unset_due_date));

    g_signal_connect(adw_dialog->gobj(), "closed",
                     G_CALLBACK(+[](AdwDialog* self, gpointer data) {
                         reinterpret_cast<CardDetailsDialog*>(data)->on_save();
                         reinterpret_cast<CardDetailsDialog*>(data)->clear();

                         spdlog::get("ui")->info("Card Dialog closed");
                     }),
                     this);

    tasks_box->append(checklist_add_button);
}

CardDetailsDialog::~CardDetailsDialog() {}

void CardDetailsDialog::remove_task(TaskWidget& task) {
    auto card = cur_card_widget->get_card();
    card->remove(*task.get_task());
    tasks_box->remove(task);
    std::erase(tasks_tracker, &task);
    cur_card_widget->update_complete_tasks();

    spdlog::get("ui")->debug("TaskWidget \"{}\" removed from the Card Dialog",
                             task.get_task()->get_name());
}

void CardDetailsDialog::reorder_task_widget(TaskWidget& next,
                                            TaskWidget& sibling) {
    tasks_box->reorder_child_after(next, sibling);
    cur_card_widget->get_card()->reorder(*next.get_task(), *sibling.get_task());

    spdlog::get("ui")->debug(
        "TaskWidget \"{}\" reordered after TaskWidget \"{}\"",
        next.get_task()->get_name(), sibling.get_task()->get_name());
}

void CardDetailsDialog::open(Gtk::Window& parent, CardWidget* card_widget) {
    // Load the card contents into the dialog
    cur_card_widget = card_widget;
    auto card_ptr = this->cur_card_widget->get_card();

    if (card_ptr->get_due_date().ok()) {
        update_due_date_label();
        checkbutton_revealer->set_reveal_child(true);
        checkbutton->set_active(card_ptr->get_complete());
    }

    card_title_entry->set_text(card_ptr->get_name());
    for (auto& task : card_ptr->get_tasks()) {
        _add_task(task);
    }
    notes_textbuffer->set_text(card_ptr->get_notes());

    adw_dialog_present(ADW_DIALOG(adw_dialog->gobj()),
                       static_cast<Gtk::Widget&>(parent).gobj());

    spdlog::get("app")->info("Card dialog opened for Card {}",
                             card_widget->get_card()->get_name());
}

void CardDetailsDialog::close() {
    adw_dialog_close(ADW_DIALOG(adw_dialog->gobj()));
}

void CardDetailsDialog::update_due_date_label() {
    auto sys_days =
        std::chrono::sys_days(cur_card_widget->get_card()->get_due_date());
    std::time_t time = std::chrono::system_clock::to_time_t(sys_days);
    char date_str[255];
    strftime(date_str, 255, "%x", std::gmtime(&time));
    date_menubutton->set_label(Glib::ustring{date_str});

    spdlog::get("ui")->debug("Card Dialog's due date label has been updated");
}

CardWidget* CardDetailsDialog::get_card_widget() { return cur_card_widget; }

void CardDetailsDialog::on_add_task() {
    _add_task(cur_card_widget->get_card()->add(Task{_("New Task")}), true);
    cur_card_widget->update_complete_tasks();
}

void CardDetailsDialog::on_save() {
    if (!cur_card_widget) {
        return;
    }

    auto card = cur_card_widget->get_card();
    std::string new_card_name = card_title_entry->get_text();
    std::string new_notes = notes_textbuffer->get_text();

    if (card->get_name() != new_card_name) {
        cur_card_widget->set_title(new_card_name);
        card->set_name(new_card_name);
    }

    if (card->get_notes() != new_notes) {
        card->set_notes(new_notes);
    }

    cur_card_widget->set_tooltip_text(cur_card_widget->create_details_text());

    spdlog::get("ui")->info("Card dialog has saved changes made to Card \"{}\"",
                            card->get_name());
}

void CardDetailsDialog::on_delete_card() {
    cur_card_widget->remove_from_parent();
    cur_card_widget = nullptr;

    spdlog::get("ui")->info("Card dialog has deleted a card");

    close();
}

void CardDetailsDialog::on_unset_due_date() {
    auto card_ptr = cur_card_widget->get_card();

    date_menubutton->set_label(_("Set Due Date"));
    card_ptr->set_due_date(Date{});
    checkbutton_revealer->set_reveal_child(false);
    cur_card_widget->update_due_date();

    spdlog::get("ui")->debug("Card Dialog just had a due date unset");
}

void CardDetailsDialog::on_set_due_date() {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto card_ptr = cur_card_widget->get_card();

    int y, m, d;
    calendar->get_date().get_ymd(y, m, d);
    auto new_date = Date{year{y}, month{static_cast<unsigned int>(m)},
                         day{static_cast<unsigned int>(d)}};
    card_ptr->set_due_date(new_date);
    checkbutton_revealer->set_reveal_child(true);
    cur_card_widget->update_due_date();
    update_due_date_label();

    spdlog::get("ui")->debug("Card Dialog just had a due date set");
}

void CardDetailsDialog::clear() {
    card_title_entry->set_text("");
    for (TaskWidget* task_widget : tasks_tracker) {
        tasks_box->remove(*task_widget);
    }
    tasks_tracker.clear();
    notes_textbuffer->set_text("");

    date_menubutton->set_label(_("Set Due Date"));
    checkbutton_revealer->set_reveal_child(false);

    cur_card_widget = nullptr;

    spdlog::get("ui")->debug("Card Dialog set to an empty state");
}

void CardDetailsDialog::_add_task(const std::shared_ptr<Task> task,
                                  bool is_new) {
    auto task_widget =
        Gtk::make_managed<TaskWidget>(*this, *cur_card_widget, task, is_new);
    tasks_box->append(*task_widget);
    tasks_box->reorder_child_after(checklist_add_button, *task_widget);
    tasks_tracker.push_back(task_widget);

    spdlog::get("ui")->debug(
        "TaskWidget \"{}\" has been added to the Card Dialog's task list",
        task->get_name());
}
}  // namespace ui
