#pragma once

#include <string>

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
    Item(const std::string& name);

    virtual ~Item() = default;

    /**
     * @brief Changes the object's name.
     *
     * @param other New name.
     */
    virtual void set_name(const std::string& other);

    /**
     * @brief Sets modified state
     */
    virtual void set_modified(bool modified);

    /**
     * @brief Gets the name of the object.
     *
     * @returns String as the name of the object.
     */
    virtual std::string get_name() const;

    /**
     * @brief Produces true if the Item object was modified.
     */
    virtual bool get_modified();

    /**
     * @brief Gets the object's ID.
     *
     * @returns The integer as the object's ID.
     */
    virtual unsigned long long get_id() const;

    bool operator==(const Item& Item) const;

protected:
    std::string name;
    unsigned long long id;
    bool modified = false;
};
