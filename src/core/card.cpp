#include "card.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <numeric>

Card::Card(const std::string& name, const Date& date, bool complete,
           const Color& color)
    : Item{name, xg::newGuid()}, complete{complete}, due_date{date} {
    this->color = color;
}

Card::Card(const std::string& name, const Date& date, const xg::Guid uuid,
           bool complete, const Color& color)
    : Item{name}, complete{complete}, due_date{date} {
    this->color = color;
}

Card::Card(const std::string& name, const Color& color)
    : Card{name, Date{}, false, color} {}

Card::Card(const std::string& name, const xg::Guid uuid, const Color& color)
    : Card{name, Date{}, uuid, false, color} {}

Card::~Card() {}

void Card::set_color(const Color& color) {
    this->color = color;
    set_modified();

    spdlog::get("core")->info("[Card] Card \"{}\"'s color set to: {}", name,
                              color_to_string(color));
}

const std::string& Card::get_notes() const { return notes; }

bool Card::get_modified() const { return modified || tasks.get_modified(); }

void Card::set_notes(const std::string& notes) {
    this->notes = notes;
    set_modified();
    spdlog::get("core")->info("[Card] Card \"{}\" notes set to: \"{}\"", name,
                              notes);
}

double Card::get_completion() const {
    double tasks_completed_n =
        std::accumulate(tasks.begin(), tasks.end(), 0,
                        [](double acc, std::shared_ptr<Task> value) {
                            return value->get_done() ? acc + 1 : acc;
                        });

    return (tasks_completed_n * 100) / tasks.get_data().size();
}

bool Card::past_due_date() {
    using namespace std::chrono;

    Date today = year_month_day{std::chrono::floor<days>(system_clock::now())};
    return (due_date.ok()) && today > due_date;
};

void Card::set_due_date(const Date& date) {
    due_date = date;
    set_modified(true);
    spdlog::get("core")->info("[Card] Card \"{}\" due date set to: {}", name,
                              std::format("{}", date));
};

Date Card::get_due_date() const { return due_date; };

bool Card::get_complete() const { return due_date.ok() ? complete : true; }

void Card::set_complete(bool complete) {
    if (due_date.ok()) {
        this->complete = complete;
        set_modified(true);
        spdlog::get("core")->info("[Card] Card \"{}\" marked as {}", name,
                                  (complete ? "complete" : "incomplete"));
    } else {
        spdlog::get("core")->warn(
            "[Card] Card \"{}\" cannot be set as complete because no deadline "
            "was assigned",
            name);
    }
}

ItemContainer<Task>& Card::container() { return tasks; }
