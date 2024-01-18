#pragma once

#include <vector>
#include <memory>

#include "card.h"
#include "item.h"

/**
 * @class CardList
 *
 * @brief A class representing a single list of cards inside a board.
 */
class CardList : public Item {
public:
    /**
     * @brief CardList constructor;
     */
    CardList(std::string name);

    /**
     * @brief Adds a Card object to the cardlist by moving the Card object
     * into a newly allocated space.
     *
     * @returns A pointer to the allocated object. It may be nullptr
     */
    std::shared_ptr<Card> add_card(Card& card);

    /**
     * @brief Adds a Card object to the cardlist by moving the Card object
     * into a newly allocated space.
     *
     * @returns A pointer to the allocated object. It may be nullptr
     */
    std::shared_ptr<Card> add_card(Card&& card);

    /**
     * @brief Removes a Card object from the cardlist.
     *
     * @param card Card instance.
     *
     * @returns True if the card was successfully removed from the cardlist.
     *          False may be returned when the requested Card to be removed is
     *          not present in the cardlist.
     */
    bool remove_card(Card& card);

    /**
     * @brief Creates a pointer to a constant vector object.
     *
     * @param card \ref Card instance.
     *
     * @returns the pointer pointing to a constant vector object.
     */
    std::vector<std::shared_ptr<Card>>& get_card_vector();

    void reorder_card(std::shared_ptr<Card> next, std::shared_ptr<Card> sibling);

private:
    std::vector<std::shared_ptr<Card>> card_vector;
};