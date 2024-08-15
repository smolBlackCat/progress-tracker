#include "card.h"

#include <numeric>

Card::Card(const std::string& name, const Gdk::RGBA& color)
    : Item{name}, color{color} {}

void Card::set_color(const Gdk::RGBA& color) {
    this->color = color;
    modified = true;
}

Gdk::RGBA Card::get_color() const { return color; }

bool Card::is_color_set() { return color != NO_COLOR; }

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
    if (tasks.size() == 0) {
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
