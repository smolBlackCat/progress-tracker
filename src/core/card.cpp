#include "card.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <numeric>
#include <utility>

Card::Card(const std::string& name, const Date& date, bool complete,
           const Color& color)
    : Item{name, xg::newGuid()}, m_complete{complete}, m_due_date{date} {
    this->color = color;
}

Card::Card(const std::string& name, const Date& date, const xg::Guid uuid,
           bool complete, const Color& color)
    : Item{name, uuid}, m_complete{complete}, m_due_date{date} {
    this->color = color;
}

Card::Card(const std::string& name, const Color& color)
    : Card{name, Date{}, false, color} {}

Card::Card(const std::string& name, const xg::Guid uuid, const Color& color)
    : Card{name, Date{}, uuid, false, color} {}

Card::~Card() {}

void Card::set_name(const std::string& name) {
    Item::set_name(name);
    modify();
}

void Card::set_color(const Color& color) {
    Color old_color = color;
    this->color = color;
    modify();

    spdlog::get("core")->info("[Card] Card \"{}\"'s color set to: {}", name,
                              color_to_string(color));

    color_signal.emit(std::move(old_color), this->color);
}

const std::string& Card::get_notes() const { return m_notes; }

bool Card::modified() const { return m_modified || m_tasks.modified(); }

void Card::set_notes(const std::string& notes) {
    std::string old_notes = m_notes;
    this->m_notes = notes;
    modify();
    spdlog::get("core")->info("[Card] Card \"{}\" notes set to: \"{}\"", name,
                              notes);
    notes_signal.emit(std::move(old_notes), notes);
}

double Card::get_completion() const {
    double tasks_completed_n =
        std::accumulate(m_tasks.begin(), m_tasks.end(), 0,
                        [](double acc, std::shared_ptr<Task> value) {
                            return value->get_done() ? acc + 1 : acc;
                        });

    return (tasks_completed_n * 100) / m_tasks.get_data().size();
}

bool Card::past_due_date() {
    using namespace std::chrono;

    Date today = year_month_day{std::chrono::floor<days>(system_clock::now())};
    return (m_due_date.ok()) && today > m_due_date;
};

void Card::set_due_date(const Date& date) {
    Date old = m_due_date;
    m_due_date = date;
    due_date_signal.emit(std::move(old), m_due_date);
    modify();
    spdlog::get("core")->info("[Card] Card \"{}\" due date set to: {}", name,
                              std::format("{}", date));
};

void Card::modify(bool m) { m_modified = m; }

Date Card::get_due_date() const { return m_due_date; };

bool Card::get_complete() const { return m_due_date.ok() ? m_complete : true; }

void Card::set_complete(bool complete) {
    if (m_due_date.ok()) {
        this->m_complete = complete;
        modify();
        spdlog::get("core")->info("[Card] Card \"{}\" marked as {}", name,
                                  (complete ? "complete" : "incomplete"));
        complete_signal.emit(m_complete);
    } else {
        spdlog::get("core")->warn(
            "[Card] Card \"{}\" cannot be set as complete because no deadline "
            "was assigned",
            name);
    }
}

ItemContainer<Task>& Card::container() { return m_tasks; }

sigc::signal<void(Color, Color)>& Card::signal_color() { return color_signal; }

sigc::signal<void(std::string, std::string)>& Card::signal_notes() {
    return notes_signal;
}

sigc::signal<void(Date, Date)>& Card::signal_due_date() {
    return due_date_signal;
}

sigc::signal<void(bool)>& Card::signal_complete() { return complete_signal; }