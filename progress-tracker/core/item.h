#pragma once

#include <string>
#include "tinyxml2.h"

class Item {

public:
    Item(std::string name);

    void set_name(std::string other);
    std::string get_name() const;
    bool operator==(const Item& Item) const;
    unsigned long long get_id() const;

protected:
    std::string name;
    unsigned long long id;
};