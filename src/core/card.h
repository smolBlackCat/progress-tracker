#pragma once

#include "colorable.h"
#include "item.h"
#include "task.h"

/**
 * @class Card
 *
 * @brief A class representing a single card within a \ref CardList object.
 */
class Card : public Colorable, public Item {
public:
    /**
     * @brief Card constructor.
     *
     * @param name The card's title.
     */
    Card(const std::string& name, const Gdk::RGBA& color = NO_COLOR);

    void set_color(const Gdk::RGBA& color) override;

    Gdk::RGBA get_color() const override;

    bool is_color_set() override;

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
     * either when the card does not have any tasks added or no tasks was
     * completed at all
     */
    double get_completion() const;

    /**
     * @brief Adds a task to this card.
     *
     * @param task Task object to be added
     *
     * @return a smart pointer to the the added Task object
     */
    std::shared_ptr<Task> add_task(const Task& task);

    /**
     * @brief Removes a Task object
     *
     * @param task pointer to the task object to be removed
     *
     * @return True when the object was removed. False when the Task object to
     * be removed was not even in the list
     */
    bool remove_task(std::shared_ptr<Task> task);

    /**
     * @brief Access the underlying Task collection
     */
    std::vector<std::shared_ptr<Task>> const& get_tasks();

    void reorder_task(const Task& next, const Task& sibling);

protected:
    Gdk::RGBA color = NO_COLOR;
    std::string notes;
    std::vector<std::shared_ptr<Task>> tasks;
};