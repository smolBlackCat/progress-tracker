#pragma once

#include <chrono>

#include "colorable.h"
#include "item-container.h"
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

    Card(const std::string& name, const Date& date, const xg::Guid uuid,
         bool complete = false, const Color& color = NO_COLOR);

    /**
     * @brief Card constructor
     */
    Card(const std::string& name, const Color& color = NO_COLOR);

    /**
     * @brief Card constructor
     */
    Card(const std::string& name, const xg::Guid uuid,
         const Color& color = NO_COLOR);

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

    bool get_modified() const override;

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

    ItemContainer<Task>& container();

protected:
    std::string notes;
    ItemContainer<Task> tasks;
    Date due_date;
    bool complete;
};