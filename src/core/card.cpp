#include "card.h"

#include <numeric>

Card::Card(const std::string& name, const Date& date, bool complete,
           const Color& color)
    : Item{name}, complete{complete}, due_date{date} {
    this->color = color;
}

Card::Card(const std::string& name, const Color& color)
    : Card{name, Date{}, false, color} {}

void Card::set_color(const Color& color) {
    this->color = color;
    modified = true;
}

const std::string& Card::get_notes() const { return notes; }

void Card::set_modified(bool modified) {
    Item::set_modified(modified);
    for (auto& task : tasks) {
        task->set_modified(modified);
    }
}

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
}

double Card::get_completion() const {
    double tasks_completed_n =
        std::accumulate(tasks.begin(), tasks.end(), 0,
                        [](double acc, std::shared_ptr<Task> value) {
                            return value->get_done() ? acc + 1 : acc;
                        });

    return (tasks_completed_n * 100) / tasks.size();
}

std::shared_ptr<Task> Card::add_task(const Task& task) {
    for (auto& ctask : tasks) {
        if (task == *ctask) return nullptr;
    }

    std::shared_ptr<Task> new_task = std::make_shared<Task>(task);
    tasks.push_back(new_task);

    modified = true;

    return new_task;
}

bool Card::remove_task(const Task& task) {
    for (size_t i = 0; i < tasks.size(); i++) {
        if (*tasks[i] == task) {
            tasks.erase(tasks.begin() + i);
            modified = true;
            return true;
        }
    }
    return false;
}

std::vector<std::shared_ptr<Task>> const& Card::get_tasks() { return tasks; }

void Card::reorder_task(const Task& next, const Task& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    for (ssize_t i = 0; i < tasks.size(); i++) {
        if (*tasks[i] == next) {
            next_i = i;
        }
        if (*tasks[i] == sibling) {
            sibling_i = i;
        }
    }

    bool any_absent_item = next_i + sibling_i < 0;
    bool is_same_item = next_i == sibling_i;
    bool already_in_order = next_i - sibling_i == 1;
    if (any_absent_item || is_same_item || already_in_order) {
        return;
    }

    auto next_it = std::next(tasks.begin(), next_i);
    std::shared_ptr<Task> temp_v = tasks[next_i];
    tasks.erase(next_it);

    // Support for right to left drags and drops
    if (next_i < sibling_i) {
        sibling_i -= 1;
    }

    if (sibling_i == tasks.size() - 1) {
        tasks.push_back(temp_v);
    } else {
        auto sibling_it = std::next(tasks.begin(), sibling_i + 1);
        tasks.insert(sibling_it, temp_v);
    }
    modified = true;
}

bool Card::past_due_date() {
    using namespace std::chrono;

    Date today = year_month_day{std::chrono::floor<days>(system_clock::now())};
    return (due_date.ok()) && today > due_date;
};

void Card::set_due_date(const Date& date) {
    if (date.ok()) {
        due_date = date;
        modified = true;
    }
};

Date Card::get_due_date() const { return due_date; };

bool Card::get_complete() const { return due_date.ok() ? complete : true; }

void Card::set_complete(bool complete) {
    if (due_date.ok()) {
        this->complete = complete;
        modified = true;
    }
}
