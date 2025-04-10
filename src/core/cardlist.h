#pragma once

#include <memory>
#include <vector>

#include "card.h"
#include "item-container.h"
#include "item.h"

/**
 * @brief Represents a list of cards within a kanban board
 */
class CardList : public Item, public ItemContainer<Card> {
public:
    /**
     * @brief CardList constructor;
     */
    CardList(const std::string& name);

    CardList(const std::string& name, const xg::Guid uuid);

    void set_modified(bool modified) override;
    bool get_modified() const;

protected:
    std::vector<std::shared_ptr<Card>> cards;
};