#pragma once

#include <string>

class Item {

public:
    Item(std::string name);
    Item(std::string name, unsigned long long id);

    void set_name(std::string other);
    std::string get_name() const;
    bool operator==(const Item& Item) const;
    unsigned long long get_id() const;

    virtual std::string fetch_xml() const;

protected:
    std::string name;
    unsigned long long id;
};