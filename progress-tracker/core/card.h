#pragma once

#include "item.h"

class Card : public Item {

public:
    Card(std::string name);
    Card(std::string name, unsigned long long id);
};