#pragma once

#include <chrono>

#include "colorable.h"
#include "item-container.h"
#include "item.h"
#include "modifiable.h"
#include "task.h"

typedef std::chrono::year_month_day Date;

/**
 * @brief Represents a kanban card that may or may not be contained within a
 * CardList object
 */
class Card : public Colorable, public Item, public Modifiable {
public:
    /**
     * @brief Card constructor
     *
     * @param name name of the card
     * @param date deadline date of the card
     * @param complete completion status of the card
     * @param color color of the card
     */
    Card(const std::string& name, const Date& date, bool complete = false,
         const Color& color = NO_COLOR);

    /**
     * @brief Card constructor
     *
     * @param name name of the card
     * @param date deadline date of the card
     * @param uuid unique identifier of the card
     * @param complete completion status of the card
     * @param color color of the card
     */
    Card(const std::string& name, const Date& date, const xg::Guid uuid,
         bool complete = false, const Color& color = NO_COLOR);

    /**
     * @brief Card constructor
     *
     * @param name name of the card
     * @param color color of the card
     */
    Card(const std::string& name, const Color& color = NO_COLOR);

    /**
     * @brief Card constructor
     *
     * @param name name of the card
     * @param uuid unique identifier of the card
     * @param color color of the card
     */
    Card(const std::string& name, const xg::Guid uuid,
         const Color& color = NO_COLOR);

    ~Card() override;

    void set_name(const std::string& name) override;

    /**
     * @brief Sets the card's cover color
     *
     * @param rgb Color (std::tuple) object representing the rgb code
     */
    void set_color(const Color& rgb) override;

    /**
     * @brief Update the notes associated with this card
     *
     * @param notes The new notes
     */
    void set_notes(const std::string& notes);

    /**
     * @brief Sets a new due date to this card. This method will not modify due
     * date if the given date is not valid
     */
    void set_due_date(const Date& date);

    /**
     * @brief Sets the complete state of this card. This method does nothing
     * whenever the due date set is not valid
     *
     * @param complete bool value
     */
    void set_complete(bool complete);

    void modify(bool m = true) override;

    /**
     * @brief Return the notes associated with this card
     */
    const std::string& get_notes() const;

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
     * @brief Returns card's due date
     */
    Date get_due_date() const;

    /**
     * @brief Produces true if the card information has been modified or the
     * card's container has been modified
     */
    bool modified() const override;

    /**
     * @brief Returns true if this card is past due date and the date is valid,
     * else false is returned
     */
    bool past_due_date();

    /**
     * @brief Return true if the card is complete or an invalid due date is set.
     * False is then returned if the due date set is valid but it is not
     * complete
     */
    bool get_complete() const;

    /**
     * @brief Returns a reference to the container of tasks associated with this
     * card
     */
    ItemContainer<Task>& container();

    sigc::signal<void(Color, Color)>& signal_color();
    sigc::signal<void(std::string, std::string)>& signal_notes();
    sigc::signal<void(Date, Date)>& signal_due_date();
    sigc::signal<void(bool)>& signal_complete();

protected:
    std::string m_notes;
    ItemContainer<Task> m_tasks;
    Date m_due_date;
    bool m_complete;
    bool m_modified = false;

    // Signals
    sigc::signal<void(Color, Color)> color_signal;  // f(old_colour, new_colour)
    sigc::signal<void(std::string, std::string)>
        notes_signal;                                // f(old_notes, new_notes)
    sigc::signal<void(Date, Date)> due_date_signal;  // f(old_date, new_date)
    sigc::signal<void(bool)> complete_signal;        // f(cur_state)
};