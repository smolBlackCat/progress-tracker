#pragma once

#include "item.h"

class Task : public Item {
public:
    Task(const std::string& name, bool done = false);

    bool get_done() const;
    void set_done(bool done = true);

protected:
    bool done;
};