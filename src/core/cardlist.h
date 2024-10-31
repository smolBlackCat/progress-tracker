#pragma once

#include <memory>
#include <vector>

#include "card.h"
#include "item.h"

/**
 * @brief Represents a list of cards within a kanban board
 */
class CardList : public Item {
public:
    /**
     * @brief CardList constructor;
     */
    CardList(const std::string& name);

    /**
     * @brief Adds a Card object to the cardlist by copying the Card object
     * into a newly allocated space.
     *
     * @returns A pointer to the allocated object. It may be nullptr if the card
     * being added is already in the cardlist
     */
    std::shared_ptr<Card> add(const Card& card);

    /**
     * @brief Removes a Card object from the cardlist.
     *
     * @param card Card instance.
     *
     * @returns True if the card was removed from the cardlist.
     *          False may be returned when the requested Card to be removed is
     *          not present in the cardlist.
     */
    bool remove(const Card& card);

    void set_modified(bool modified) override;

    bool get_modified() override;

    /**
     * @brief Access the underlying cards collection
     */
    const std::vector<std::shared_ptr<Card>>& get_cards();

    /**
     * @brief Reorders card "next" after card "sibling"
     */
    void reorder(const Card& next, const Card& sibling);

private:
    bool cards_modified();

protected:
    std::vector<std::shared_ptr<Card>> cards;
};