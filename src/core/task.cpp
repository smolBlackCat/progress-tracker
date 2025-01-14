#include "task.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

Task::Task(const std::string& name, bool done) : Item{name}, done{done} {}

bool Task::get_done() const { return done; }

void Task::set_done(bool done) {
    this->done = done;
    modified = true;
    spdlog::get("core")->info("Task \"{}\" done set to: {}", name, done);
}
