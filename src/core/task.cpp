#include "task.h"

Task::Task(const std::string& name, bool done)
    : Item{name, xg::newGuid()}, m_done{done} {}

Task::Task(const std::string& name, const xg::Guid uuid, bool done)
    : Item{name, uuid}, m_done{done} {}

bool Task::get_done() const { return m_done; }

bool Task::modified() const { return m_modified; }

void Task::set_name(const std::string& name) {
    Item::set_name(name);
    modify();
}

void Task::set_done(bool done) {
    m_done = done;
    m_modified = true;

    done_signal.emit(done);
}

void Task::modify(bool m) { m_modified = m; }

sigc::signal<void(bool)>& Task::signal_done() { return done_signal; }
