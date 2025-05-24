#pragma once
#include <sigc++/signal.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <guid.hpp>
#include <string>

extern const std::shared_ptr<spdlog::logger> core_logger;

enum class ReorderingType {
    UPDOWN,
    DOWNUP,
    INVALID,
};

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
    Item(const std::string& name, xg::Guid uuid);

    virtual ~Item();

    /**
     * @brief Changes the object's name.
     *
     * @param other New name.
     */
    virtual void set_name(const std::string& other);

    /**
     * @brief Gets the name of the object.
     *
     * @returns String as the name of the object.
     */
    virtual std::string get_name() const;

    /**
     * @brief Gets the object's ID.
     *
     * @returns The integer as the object's ID.
     */
    virtual xg::Guid get_id() const;

    /**
     * @brief Signal emitted when the item's name is changed.
     *
     * @returns Reference to the signal.
     */
    sigc::signal<void()>& signal_name_changed();

    bool operator==(const Item& Item) const;

protected:
    std::string name;
    xg::Guid uuid;

    // Signals
    sigc::signal<void()> name_changed;
};
