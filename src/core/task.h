#pragma once

#include "item.h"
#include "modifiable.h"

/**
 * @brief Class representing an extra task associated with a card
 */
class Task : public Item, public Modifiable {
public:
    /**
     * @brief Task constructor
     *
     * @param name The name of the task
     * @param done Whether the task is done or not
     */
    Task(const std::string& name, bool done = false);

    /**
     * @brief Task constructor
     *
     * @param name The name of the task
     * @param uuid The unique identifier of the task
     * @param done Whether the task is done or not
     */
    Task(const std::string& name, const xg::Guid uuid, bool done = false);

    void set_name(const std::string& name) override;

    /**
     * @brief Set the done attribute to this card. Calling this method with no
     * arguments mark this task as done by default
     */
    void set_done(bool done = true);

    /**
     * @brief Sets this task as modified
     */
    void modify(bool m = true) override;

    /**
     * @brief Returns true if the card is marked as done, otherwise false
     */
    bool get_done() const;

    /**
     * @brief Returns the Task's modified state
     */
    bool modified() const override;

    sigc::signal<void(bool)>& signal_done();

protected:
    bool m_done;
    bool m_modified = false;

    // Signals
    sigc::signal<void(bool)> done_signal;
};