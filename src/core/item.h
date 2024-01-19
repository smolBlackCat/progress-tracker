#pragma once

#include <string>

#include "tinyxml2.h"

/**
 * @class Item
 *
 * @brief Base class for representing items of a Kanban-style todo list.
 */
class Item {
public:
    /**
     * @brief Base class Constructor.
     *
     * @param name The item's name.
     */
    Item(std::string name);

    virtual ~Item() = default;

    /**
     * @brief Changes the object's name.
     *
     * @param other New name.
     */
    void set_name(std::string other);

    /**
     * @brief Gets the name of the object.
     *
     * @returns String as the name of the object.
     */
    std::string get_name() const;

    /**
     * @brief Gets the object's ID.
     *
     * @returns The integer as the object's ID.
     */
    unsigned long long get_id() const;

    bool operator==(const Item& Item) const;

protected:
    std::string name;
    unsigned long long id;
};