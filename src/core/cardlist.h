#pragma once

#include <memory>
#include <vector>

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
    CardList(const std::string& name);

    /**
     * @brief Adds a Card object to the cardlist by moving the Card object
     * into a newly allocated space.
     *
     * @returns A pointer to the allocated object. It may be nullptr
     */
    std::shared_ptr<Card> add_card(const Card& card);

    /**
     * @brief Removes a Card object from the cardlist.
     *
     * @param card Card instance.
     *
     * @returns True if the card was successfully removed from the cardlist.
     *          False may be returned when the requested Card to be removed is
     *          not present in the cardlist.
     */
    bool remove_card(const Card& card);

    void set_modified(bool modified) override;

    bool get_modified() override;

    const std::vector<std::shared_ptr<Card>>& get_card_vector();

    /**
     * @brief Reorders the cards in a way that next is put after sibling.
     *
     * @param next Card to be put after sibling
     * @param sibling Card where next will be put after
     *
     * @throws std::invalid_argument if either next or sibling are not children
     * of this cardlist
     */
    void reorder_card(std::shared_ptr<Card> next,
                      std::shared_ptr<Card> sibling);

private:
    bool cards_modified();

protected:
    std::vector<std::shared_ptr<Card>> card_vector;
};