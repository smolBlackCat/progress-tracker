#include "task.h"

Task::Task(const std::string& name, bool done) : Item{name}, done{done} {}

bool Task::get_done() const { return done; }

void Task::set_done(bool done) {
    this->done = done;
    modified = true;
}
