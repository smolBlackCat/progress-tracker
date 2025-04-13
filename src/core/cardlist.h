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
     */
    CardList(const std::string& name);

    CardList(const std::string& name, const xg::Guid uuid);

    bool get_modified() const override;

    ItemContainer<Card>& container();

protected:
    ItemContainer<Card> cards;
};
