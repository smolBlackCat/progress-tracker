#include "item.h"

#include <chrono>
#include <random>

Item::Item(std::string name) : name{name} {
    std::default_random_engine gen(
        (long)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<unsigned long long> distribution(1,
                                                                   INT64_MAX);
    id = distribution(gen);
}

void Item::set_name(std::string other) { name = other; }

std::string Item::get_name() const { return name; }

bool Item::operator==(const Item& item) const {
    return item.get_name() == name && item.id == id;
}

unsigned long long Item::get_id() const { return id; }