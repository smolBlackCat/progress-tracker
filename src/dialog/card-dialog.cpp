#include "card-dialog.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <widgets/card-widget.h>
#include <widgets/task-widget.h>

#include <ctime>

#include "glibmm/ustring.h"

namespace ui {

CardDetailsDialog::CardDetailsDialog()
    : m_checklist_add_button{_("Add Task")},
      builder{Gtk::Builder::create_from_resource(
          "/io/github/smolblackcat/Progress/card-details-dialog.ui")},
      m_title_entry{builder->get_widget<Gtk::Entry>("card-title-entry")},
      m_unset_due_date_button{
          builder->get_widget<Gtk::Button>("unset-due-date-button")},
      m_card_delete_button{
          builder->get_widget<Gtk::Button>("delete-card-button")},
      m_date_menubutton{
          builder->get_widget<Gtk::MenuButton>("date-menubutton")},
      m_calendar{builder->get_widget<Gtk::Calendar>("calendar")},
      m_checkbutton_revealer{
          builder->get_widget<Gtk::Revealer>("checkbutton-revealer")},
      m_checkbutton{builder->get_widget<Gtk::CheckButton>("checkbutton")},
      m_tasks_box{builder->get_widget<Gtk::Box>("tasks-box")},
      m_notes_textbuffer{
          builder->get_object<Gtk::TextBuffer>("notes-textbuffer")},
      m_adw_dialog{builder->get_object("card-dialog")} {
    // Signal connections
    m_checklist_add_button.signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_add_task));
    m_card_delete_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_delete_card));
    m_unset_due_date_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_unset_due_date));

    m_checkbutton->signal_toggled().connect([this]() {
        this->m_card_widget->get_card()->set_complete(
            this->m_checkbutton->get_active());
    });
    m_calendar->signal_day_selected().connect(
        sigc::mem_fun(*this, &CardDetailsDialog::on_set_due_date));

    g_signal_connect(m_adw_dialog->gobj(), "closed",
                     G_CALLBACK(+[](AdwDialog* self, gpointer data) {
                         reinterpret_cast<CardDetailsDialog*>(data)
                             ->get_card_widget()
                             ->grab_focus();
                         reinterpret_cast<CardDetailsDialog*>(data)->on_save();
                         reinterpret_cast<CardDetailsDialog*>(data)->clear();

                         spdlog::get("app")->info("Card dialog closed");
                     }),
                     this);

    m_checklist_add_button.set_margin_start(4);
    m_checklist_add_button.set_margin_end(4);
    m_checklist_add_button.set_margin_bottom(3);
    m_tasks_box->append(m_checklist_add_button);
}

CardDetailsDialog::~CardDetailsDialog() {}

TaskWidget* CardDetailsDialog::add_task(const Task& task) {
    TaskWidget* task_widget =
        _add_task(m_card_widget->get_card()->container().append(task), false);
    spdlog::get("app")->info("Task (\"{}\") added to card (\"{}\")",
                             task.get_name(),
                             m_card_widget->get_card()->get_name());

    return task_widget;
}

TaskWidget* CardDetailsDialog::insert_new_task_after(const Task& task,
                                                     TaskWidget* sibling) {
    auto new_task = m_card_widget->get_card()->container().insert_after(
        task, *sibling->task());
    auto task_widget = Gtk::make_managed<TaskWidget>(*this, new_task, false);
    m_tasks_box->insert_child_after(*task_widget, *sibling);
    m_tasks_tracker.push_back(task_widget);
    m_card_widget->update_complete_tasks_label();

    return task_widget;
}

void CardDetailsDialog::remove_task(TaskWidget& task_widget) {
    auto card = m_card_widget->get_card();
    auto task = task_widget.task();

    if (Gtk::Widget* previous_sibling = task_widget.get_prev_sibling()) {
        previous_sibling->grab_focus();
    }

    card->container().remove(*task);
    m_tasks_box->remove(task_widget);
    std::erase(m_tasks_tracker, &task_widget);

    spdlog::get("app")->info("Task (\"{}\") removed from Card (\"{}\")",
                             task->get_name(), card->get_name());
}

void CardDetailsDialog::reorder_task_widget(TaskWidget& next,
                                            TaskWidget& sibling) {
    ReorderingType reordering = m_card_widget->get_card()->container().reorder(
        *next.task(), *sibling.task());

    switch (reordering) {
        case ReorderingType::DOWNUP: {
            auto sibling_sibling = sibling.get_prev_sibling();
            if (!sibling_sibling) {
                m_tasks_box->reorder_child_at_start(next);
            } else {
                m_tasks_box->reorder_child_after(next, *sibling_sibling);
            }

            spdlog::get("app")->info(
                "Reordered task (\"{}\") before task (\"{}\") in Card(\"{}\")",
                next.task()->get_name(), sibling.task()->get_name(),
                m_card_widget->get_card()->get_name());
            break;
        }
        case ReorderingType::UPDOWN: {
            m_tasks_box->reorder_child_after(next, sibling);
            spdlog::get("app")->info(
                "Reordered task (\"{}\") after task (\"{}\") in Card(\"{}\")",
                next.task()->get_name(), sibling.task()->get_name(),
                m_card_widget->get_card()->get_name());
            break;
        }
        case ReorderingType::INVALID: {
            spdlog::get("ui")->warn(
                "[CardDetailsDialog.reorder] Cannot reorder (\"{}\") and "
                "(\"{}\")",
                next.task()->get_name(), sibling.task()->get_name());
            break;
        }
    }
}

void CardDetailsDialog::open(Gtk::Window& parent, CardWidget* card_widget) {
    // Load the card contents into the dialog
    m_card_widget = card_widget;
    auto card_ptr = this->m_card_widget->get_card();

    if (card_ptr->get_due_date().ok()) {
        update_due_date_label();
        m_checkbutton_revealer->set_reveal_child(true);
        m_checkbutton->set_active(card_ptr->get_complete());
    }

    m_title_entry->set_text(card_ptr->get_name());
    for (auto& task : card_ptr->container()) {
        _add_task(task);
    }
    m_notes_textbuffer->set_text(card_ptr->get_notes());

    adw_dialog_present(ADW_DIALOG(m_adw_dialog->gobj()),
                       static_cast<Gtk::Widget&>(parent).gobj());

    spdlog::get("app")->info("Card dialog opened for card (\"{}\")",
                             card_widget->get_card()->get_name());
}

void CardDetailsDialog::close() {
    adw_dialog_close(ADW_DIALOG(m_adw_dialog->gobj()));
}

void CardDetailsDialog::update_due_date_label() {
    auto sys_days =
        std::chrono::sys_days(m_card_widget->get_card()->get_due_date());
    std::time_t time = std::chrono::system_clock::to_time_t(sys_days);
    char date_str[255];
    strftime(date_str, 255, "%x", std::gmtime(&time));
    m_date_menubutton->set_label(Glib::ustring{date_str});
}

CardWidget* CardDetailsDialog::get_card_widget() { return m_card_widget; }

void CardDetailsDialog::on_add_task() {
    _add_task(
        m_card_widget->get_card()->container().append(Task{_("New Task")}),
        false);

    spdlog::get("app")->info("New task appended to Card (\"{}\")",
                             m_card_widget->get_card()->get_name());
}

void CardDetailsDialog::on_save() {
    if (!m_card_widget) {
        return;
    }

    auto card = m_card_widget->get_card();
    std::string new_card_name = m_title_entry->get_text();
    std::string new_notes = m_notes_textbuffer->get_text();

    bool changes = false;

    if (card->get_name() != new_card_name) {
        m_card_widget->set_title(new_card_name);
        card->set_name(new_card_name);
        changes = true;
    }

    if (card->get_notes() != new_notes) {
        card->set_notes(new_notes);
        changes = true;
    }

    m_card_widget->set_tooltip_markup(m_card_widget->create_details_text());

    if (changes) {
        spdlog::get("app")->info("Changes made to card (\"{}\") saved",
                                 card->get_name());
    }
}

void CardDetailsDialog::on_delete_card() {
    // Removing the card from the cardlist implies deleting it. Saves at least
    // one reference for logging purposes
    const std::shared_ptr<Card> card = m_card_widget->get_card();

    m_card_widget->remove_from_parent();
    m_card_widget = nullptr;

    close();
}

void CardDetailsDialog::on_unset_due_date() {
    auto card_ptr = m_card_widget->get_card();

    m_date_menubutton->set_label(_("Set Due Date"));
    card_ptr->set_due_date(Date{});
    m_checkbutton_revealer->set_reveal_child(false);
}

void CardDetailsDialog::on_set_due_date() {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto card_ptr = m_card_widget->get_card();

    int y, m, d;
    m_calendar->get_date().get_ymd(y, m, d);
    auto new_date = Date{year{y}, month{static_cast<unsigned int>(m)},
                         day{static_cast<unsigned int>(d)}};
    card_ptr->set_due_date(new_date);
    m_checkbutton_revealer->set_reveal_child(true);
    update_due_date_label();
}

void CardDetailsDialog::clear() {
    m_title_entry->set_text("");
    for (TaskWidget* task_widget : m_tasks_tracker) {
        m_tasks_box->remove(*task_widget);
    }
    m_tasks_tracker.clear();
    m_notes_textbuffer->set_text("");

    m_date_menubutton->set_label(_("Set Due Date"));
    m_checkbutton_revealer->set_reveal_child(false);

    m_card_widget = nullptr;

    spdlog::get("ui")->debug("[CardDetailsDialog.clear] Dialog cleaned");
}

TaskWidget* CardDetailsDialog::_add_task(const std::shared_ptr<Task>& task,
                                         bool is_new) {
    auto task_widget = Gtk::make_managed<TaskWidget>(*this, task, is_new);
    m_tasks_box->append(*task_widget);
    m_tasks_box->reorder_child_after(m_checklist_add_button, *task_widget);
    m_tasks_tracker.push_back(task_widget);
    return task_widget;
}
}  // namespace ui
