#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "colorable.h"
#include "item.h"
#include "task.h"

typedef std::chrono::year_month_day Date;

/**
 * @brief Represents a kanban card that may or may not be contained within a
 * CardList object
 */
class Card : public Colorable, public Item {
public:
    /**
     * @brief Card constructor
     */
    Card(const std::string& name, const Date& date, bool complete = false,
         const Color& color = NO_COLOR);

    /**
     * @brief Card constructor
     */
    Card(const std::string& name, const Color& color = NO_COLOR);

    ~Card() override;

    /**
     * @brief Sets the card's cover color
     *
     * @param rgb Color (std::tuple) object representing the rgb code
     */
    void set_color(const Color& rgb) override;

    /**
     * @brief Return the notes associated with this card
     */
    const std::string& get_notes() const;

    void set_modified(bool modified) override;

    bool get_modified() override;

    /**
     * @brief Update the notes associated with this card
     *
     * @param notes The new notes
     */
    void set_notes(const std::string& notes);

    /**
     * @brief Calculate the completion of this card given that it has extra
     * tasks
     *
     * @return An approximate percentage value of completion. 0 is returned
     * either when the card does not have any tasks added or no tasks were
     * completed at all
     */
    double get_completion() const;

    /**
     * @brief Adds a task to this card.
     *
     * @param task Task object to be added
     *
     * @return a smart pointer to the the added Task object. nullptr will may be
     * returned if the caller is trying to add a Task that is already in the
     * Card
     */
    std::shared_ptr<Task> add(const Task& task);

    /**
     * @brief Removes a Task object
     *
     * @param task Reference to an identical task object in tasks
     *
     * @return True when the object is removed. False is returned if task is not
     * in tasks
     */
    bool remove(const Task& task);

    /**
     * @brief Access the underlying Task collection
     */
    std::vector<std::shared_ptr<Task>> const& get_tasks();

    /**
     * @brief Put task "next" after task "sibling"
     */
    void reorder(const Task& next, const Task& sibling);

    /**
     * @brief Returns true if this card is past due date and the date is valid,
     * else false is returned
     */
    bool past_due_date();

    /**
     * @brief Sets a new due date to this card. This method will not modify due
     * date if the given date is not valid
     */
    void set_due_date(const Date& date);

    /**
     * @brief Returns card's due date
     */
    Date get_due_date() const;

    /**
     * @brief Return true if the card is complete or an invalid due date is set.
     * False is then returned if the due date set is valid but it is not
     * complete
     */
    bool get_complete() const;

    /**
     * @brief Sets the complete state of this card. This method does nothing
     * whenever the due date set is not valid
     *
     * @param complete bool value
     */
    void set_complete(bool complete);

protected:
    std::string notes;
    std::vector<std::shared_ptr<Task>> tasks;
    Date due_date;
    bool complete;
};