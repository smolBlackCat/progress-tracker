#pragma once

#include "item.h"

/**
 * @brief Task inherent to a Card
 */
class Task : public Item {
public:
    /**
     * @brief Task constructor
     */
    Task(const std::string& name, bool done = false);

    /**
     * @brief Returns true if the card is marked as done, otherwise false
     */
    bool get_done() const;

    /**
     * @brief Set the done attribute to this card. Calling this method with no
     * arguments mark this task as done by default
     */
    void set_done(bool done = true);

protected:
    bool done;
};