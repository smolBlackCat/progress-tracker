#include "card.h"

#include <numeric>

Card::Card(const std::string& name, const Color& color) : Item{name} {
    set_color(color);
    modified = false;
}

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
    if (tasks.empty()) {
        return 0;
    }

    auto tasks_completed_n =
        std::accumulate(tasks.begin(), tasks.end(), 0,
                        [](int acc, std::shared_ptr<Task> value) {
                            return value->get_done() ? acc + 1 : acc;
                        });

    return (tasks_completed_n * 100) / tasks.size();
}

std::shared_ptr<Task> Card::add_task(const Task& task) {
    std::shared_ptr<Task> new_task = std::make_shared<Task>(task);
    tasks.push_back(new_task);

    modified = true;

    return new_task;
}

bool Card::remove_task(std::shared_ptr<Task> task) {
    for (size_t i = 0; i < tasks.size(); i++) {
        if (tasks[i] == task) {
            tasks.erase(tasks.begin() + i);
            return true;
        }
    }
    return false;
}

std::vector<std::shared_ptr<Task>> const& Card::get_tasks() { return tasks; }

void Card::reorder_task(const Task& next, const Task& sibling) {
    size_t next_i = -1;
    size_t sibling_i = -1;

    for (size_t i = 0; i < tasks.size(); i++) {
        if (*tasks[i] == next) {
            next_i = i;
        }
        if (*tasks[i] == sibling) {
            sibling_i = i;
        }
    }

    if (next_i == -1 || sibling_i == -1) {
        throw std::invalid_argument{
            "Either next or sibling are not children of this cardlist"};
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
