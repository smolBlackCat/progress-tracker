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
    modified = true;

    spdlog::get("core")->info("[Card] Card \"{}\"'s color set to: {}", name,
                              color_to_string(color));
}

const std::string& Card::get_notes() const { return notes; }

bool Card::get_modified() {
    for (auto& task : tasks) {
        if (task->get_modified()) {
            return true;
        }
    }

    return modified;
}

void Card::set_notes(const std::string& notes) {
    this->notes = notes;
    modified = true;
    spdlog::get("core")->info("[Card] Card \"{}\" notes set to: \"{}\"", name,
                              notes);
}

double Card::get_completion() const {
    double tasks_completed_n =
        std::accumulate(tasks.begin(), tasks.end(), 0,
                        [](double acc, std::shared_ptr<Task> value) {
                            return value->get_done() ? acc + 1 : acc;
                        });

    return (tasks_completed_n * 100) / tasks.size();
}

std::shared_ptr<Task> Card::add(const Task& task) {
    for (auto& ctask : tasks) {
        if (task == *ctask) {
            spdlog::get("core")->warn(
                "[Card] Task \"{}\" already exists in card \"{}\"",
                task.get_name(), name);
            return nullptr;
        }
    }

    std::shared_ptr<Task> new_task = std::make_shared<Task>(task);
    tasks.push_back(new_task);

    modified = true;
    spdlog::get("core")->info("[Card] Card \"{}\" has added task \"{}\"", name,
                              task.get_name());

    return new_task;
}

bool Card::remove(const Task& task) {
    for (size_t i = 0; i < tasks.size(); i++) {
        if (*tasks[i] == task) {
            tasks.erase(tasks.begin() + i);
            modified = true;
            spdlog::get("core")->info("[Card] Card \"{}\" removed task \"{}\"",
                                      name, task.get_name());
            return true;
        }
    }
    spdlog::get("core")->warn(
        "[Card] Card \"{}\" cannot remove Task \"{}\" because it is not there",
        name, task.get_name());
    return false;
}

std::vector<std::shared_ptr<Task>> const& Card::get_tasks() { return tasks; }

ReorderingType Card::reorder(const Task& next, const Task& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    ssize_t c = 0;
    for (auto& task : tasks) {
        if (*task == next) {
            next_i = c;
        } else if (*task == sibling) {
            sibling_i = c;
        }
        c++;
    }

    bool any_absent = next_i == -1 || sibling_i == -1;
    bool is_same = next_i == sibling_i;

    if (any_absent || is_same) {
        spdlog::get("core")->warn(
            "[Card] Cannot reorder tasks: same references or missing");
        return ReorderingType::INVALID;
    }

    std::shared_ptr<Task> next_v = tasks[next_i];
    tasks.erase(tasks.begin() + next_i);

    ReorderingType reordering;

    if (next_i > sibling_i) {
        // Down to up reordering
        tasks.insert(tasks.begin() + (sibling_i == 0 ? 0 : sibling_i), next_v);
        reordering = ReorderingType::DOWNUP;
        spdlog::get("core")->info(
            "[Card] Task \"{}\" was inserted before Task \"{}\"",
            next.get_name(), sibling.get_name());
    } else if (next_i < sibling_i) {
        // Up to down reordering
        tasks.insert(tasks.begin() + sibling_i, next_v);
        reordering = ReorderingType::UPDOWN;
        spdlog::get("core")->info(
            "[Card] Task \"{}\" was inserted after Task \"{}\"",
            next.get_name(), sibling.get_name());
    }

    modified = true;
    return reordering;
}

bool Card::past_due_date() {
    using namespace std::chrono;

    Date today = year_month_day{std::chrono::floor<days>(system_clock::now())};
    return (due_date.ok()) && today > due_date;
};

void Card::set_due_date(const Date& date) {
    due_date = date;
    modified = true;
    spdlog::get("core")->info("[Card] Card \"{}\" due date set to: {}", name,
                              std::format("{}", date));
};

Date Card::get_due_date() const { return due_date; };

bool Card::get_complete() const { return due_date.ok() ? complete : true; }

void Card::set_complete(bool complete) {
    if (due_date.ok()) {
        this->complete = complete;
        modified = true;
        spdlog::get("core")->info("[Card] Card \"{}\" marked as {}", name,
                                  (complete ? "complete" : "incomplete"));
    } else {
        spdlog::get("core")->warn(
            "[Card] Card \"{}\" cannot be set as complete because no deadline "
            "was assigned",
            name);
    }
}
