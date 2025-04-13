#pragma once

#include "card.h"
#include "item.h"

/**
 * @brief Represents a list of cards within a kanban board
 */
class CardList : public Item {
public:
    /**
     * @brief CardList constructor;
     *
     * @param name name of the card list
     */
    CardList(const std::string& name);

    /**
     * @brief CardList constructor;
     *
     * @param name name of the card list
     * @param uuid unique identifier of the card list
     */
    CardList(const std::string& name, const xg::Guid uuid);

    /**
     * @brief Returns whether either the object has been modified or its
     * container has been modified
     *
     * @return true if either the object or its container has been modified
     * @return false otherwise
     */
    bool get_modified() const override;

    /**
     * @brief Returns a reference to the container of cards
     */
    ItemContainer<Card>& container();

protected:
    ItemContainer<Card> cards;
};
