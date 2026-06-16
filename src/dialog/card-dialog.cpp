#include "card-dialog.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <widgets/card-widget.h>
#include <widgets/task-widget.h>

#include <chrono>

#include "glibmm/datetime.h"
#include "glibmm/ustring.h"

namespace ui {

CardDialog::CardDialog()
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
        sigc::mem_fun(*this, &CardDialog::on_add_task));
    m_card_delete_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDialog::on_delete_card));
    m_unset_due_date_button->signal_clicked().connect(
        sigc::mem_fun(*this, &CardDialog::on_unset_due_date));

    m_checkbutton->signal_toggled().connect(
        sigc::mem_fun(m_complete_changed_signal, &sigc::signal<void()>::emit));
    m_calendar->signal_day_selected().connect(
        sigc::mem_fun(*this, &CardDialog::on_set_due_date));

    g_signal_connect(
        m_adw_dialog->gobj(), "closed",
        G_CALLBACK(+[](AdwDialog* self, gpointer data) {
            reinterpret_cast<CardDialog*>(data)->card_widget()->grab_focus();
            reinterpret_cast<CardDialog*>(data)->signal_closed().emit();
            reinterpret_cast<CardDialog*>(data)->clear();
        }),
        this);

    m_checklist_add_button.set_margin_start(4);
    m_checklist_add_button.set_margin_end(4);
    m_checklist_add_button.set_margin_bottom(3);
    m_tasks_box->append(m_checklist_add_button);
}

CardDialog::~CardDialog() {}

void CardDialog::set_title(const std::string& title) {
    if (!title.empty()) {
        const std::string old_text = m_title_entry->get_text();
        m_title_entry->set_text(title);

        m_name_changed_signal.emit(old_text, title);
    }
}

void CardDialog::set_notes(const std::string& notes) {
    const std::string old_notes = m_notes_textbuffer->get_text();
    m_notes_textbuffer->set_text(notes);

    m_notes_changed_signal.emit(old_notes, notes);
}

void CardDialog::set_deadline(const Glib::Date& deadline) {
    const Glib::DateTime old_datetime = m_calendar->get_date();
    const Glib::Date old_date(
        old_datetime.get_day_of_month(),
        static_cast<Glib::Date::Month>(old_datetime.get_month()),
        old_datetime.get_year());

    if (deadline.valid()) {
        m_date_menubutton->set_label(deadline.format_string("%x"));

        Glib::DateTime datetime = Glib::DateTime::create_local(
            deadline.get_year(), deadline.get_month_as_int(),
            deadline.get_day(), 0, 0, 0);
        m_calendar->set_date(datetime);
        m_deadline = std::chrono::year_month_day{
            std::chrono::year{int(datetime.get_year())},
            std::chrono::month{unsigned(datetime.get_month())},
            std::chrono::day{unsigned(datetime.get_day_of_month())}};
        m_checkbutton_revealer->set_reveal_child();
    } else {
        on_unset_due_date();
    }

    m_deadline_changed_signal.emit(old_date, deadline);
}

void CardDialog::set_complete(bool complete) {
    m_checkbutton->set_active(complete);
}

void CardDialog::append(TaskWidget& task) {
    m_tasks_box->append(task);
    m_tasks_box->reorder_child_after(m_checklist_add_button, task);

    task.signal_complete_changed().connect(sigc::track_obj(
        [this, &task]() { m_n_tasks_complete += task.get_complete() ? 1 : -1; },
        task));

    m_n_tasks++;
    m_n_tasks_complete += task.get_complete() ? 1 : 0;

    m_task_add_signal.emit(&task, -1);
}

// FIXME: This method (probably the similar ones too) do not have defined
// measures against non-child siblings
void CardDialog::insert_after(TaskWidget& next, TaskWidget& sibling) {
    int index = 0;
    for (Gtk::Widget* task = m_tasks_box->get_first_child();
         task && task != &sibling; task = task->get_next_sibling())
        index++;

    m_tasks_box->insert_child_after(next, sibling);

    next.signal_complete_changed().connect(sigc::track_obj(
        [this, &next]() { m_n_tasks_complete += next.get_complete() ? 1 : -1; },
        next));

    m_n_tasks++;
    m_n_tasks_complete += next.get_complete() ? 1 : 0;

    m_task_add_signal.emit(&next, index);
}

void CardDialog::remove_task(TaskWidget& task_widget) {
    if (Gtk::Widget* previous_sibling = task_widget.get_prev_sibling()) {
        previous_sibling->grab_focus();
    }

    // Whatever needs to be done with TaskWidget will be done first
    m_task_remove_signal.emit(&task_widget);

    m_n_tasks--;
    m_n_tasks_complete -= task_widget.get_complete() ? 1 : 0;

    m_tasks_box->remove(task_widget);
}

void CardDialog::reorder(TaskWidget& next, TaskWidget& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    ssize_t c_i = 0;
    for (Gtk::Widget* task = m_tasks_box->get_first_child(); task;
         task = task->get_next_sibling()) {
        if ((next_i) != -1 && (sibling_i != -1)) {
            break;
        }

        if (&next == task) {
            next_i = c_i;
        } else if (&sibling == task) {
            sibling_i = c_i;
        }
        c_i++;
    }

    if ((next_i) == -1 || (sibling_i == -1)) {
        return;
    }

    bool up = false;
    if (sibling.get_prev_sibling() == &next) {
        m_tasks_box->reorder_child_after(next, sibling);
    } else if (sibling.get_next_sibling() == &next) {
        m_tasks_box->reorder_child_after(sibling, next);
        up = true;
    } else {
        // Widgets are not neighbours. How to reorder them now depends on their
        // index
        if (next_i > sibling_i) {  // Move the widget up
            sibling.get_prev_sibling() == nullptr
                ? m_tasks_box->reorder_child_at_start(next)
                : m_tasks_box->reorder_child_after(next,
                                                   *sibling.get_prev_sibling());
            up = true;
        } else {  // Move the widget down
            m_tasks_box->reorder_child_after(next, sibling);
        }
    }

    m_task_reorder_signal.emit(&next, &sibling, up);
}

void CardDialog::open(Gtk::Window& parent, CardWidget* card_widget) {
    m_card_widget = card_widget;

    m_card_dialog_opened_signal.emit(card_widget);

    adw_dialog_present(ADW_DIALOG(m_adw_dialog->gobj()),
                       static_cast<Gtk::Widget&>(parent).gobj());
}

void CardDialog::close() { adw_dialog_close(ADW_DIALOG(m_adw_dialog->gobj())); }

CardWidget* CardDialog::card_widget() { return m_card_widget; }

sigc::signal<void(std::string, std::string)>&
CardDialog::signal_name_changed() {
    return m_name_changed_signal;
}

sigc::signal<void(std::string, std::string)>&
CardDialog::signal_notes_changed() {
    return m_notes_changed_signal;
}

sigc::signal<void(Glib::Date, Glib::Date)>&
CardDialog::signal_deadline_changed() {
    return m_deadline_changed_signal;
}

sigc::signal<void()>& CardDialog::signal_complete_changed() {
    return m_complete_changed_signal;
}

sigc::signal<void(TaskWidget*, int)>& CardDialog::signal_task_added() {
    return m_task_add_signal;
}

sigc::signal<void(TaskWidget*)>& CardDialog::signal_task_removed() {
    return m_task_remove_signal;
}

sigc::signal<void(TaskWidget*, TaskWidget*, bool)>&
CardDialog::signal_task_reordered() {
    return m_task_reorder_signal;
}

sigc::signal<void(CardWidget*)>& CardDialog::signal_open() {
    return m_card_dialog_opened_signal;
}

sigc::signal<void()>& CardDialog::signal_closed() {
    return m_card_dialog_closed_signal;
}

void CardDialog::on_add_task() {
    auto new_task = Gtk::make_managed<TaskWidget>(*this, _("New Task"));
    append(*new_task);
}

std::string CardDialog::get_title() const { return m_title_entry->get_text(); }

std::string CardDialog::get_notes() const {
    return m_notes_textbuffer->get_text();
}

std::chrono::year_month_day CardDialog::get_deadline() const {
    return m_deadline;
}

bool CardDialog::get_complete() const { return m_checkbutton->get_active(); }

std::pair<int, int> CardDialog::get_completion_ratio() const {
    return {m_n_tasks_complete, m_n_tasks};
}

void CardDialog::on_delete_card() {
    CardWidget* tmp = m_card_widget;
    close();
    tmp->remove_from_parent();
}

void CardDialog::on_unset_due_date() {
    m_date_menubutton->set_label(_("Set Due Date"));
    m_deadline = std::chrono::year_month_day{};
    m_card_widget->set_deadline_label();
    m_card_widget->set_complete(false);
    m_checkbutton_revealer->set_reveal_child(false);
}

void CardDialog::on_set_due_date() {
    auto datetime = m_calendar->get_date();
    int y, m, d;
    datetime.get_ymd(y, m, d);
    auto new_date = Glib::Date(d, static_cast<Glib::Date::Month>(m), y);
    m_deadline = std::chrono::year_month_day{std::chrono::year{y},
                                             std::chrono::month{unsigned(m)},
                                             std::chrono::day{unsigned(d)}};
    m_card_widget->set_deadline_label(new_date, get_complete());
    m_checkbutton_revealer->set_reveal_child(true);
    m_date_menubutton->set_label(new_date.format_string("%x"));
}

void CardDialog::clear() {
    m_title_entry->set_text("");
    for (Gtk::Widget* task = m_checklist_add_button.get_prev_sibling(); task;
         task = m_checklist_add_button.get_prev_sibling()) {
        m_tasks_box->remove(*task);
    }

    m_n_tasks = m_n_tasks_complete = 0;

    m_notes_textbuffer->set_text("");

    m_date_menubutton->set_label(_("Set Due Date"));
    m_checkbutton_revealer->set_reveal_child(false);

    m_card_widget = nullptr;
}
}  // namespace ui
