#pragma once

#include "item.h"

/**
 * @class Card
 *
 * @brief A class representing a single card within a \ref CardList object.
 */
class Card : public Item {
public:
    /**
     * @brief Card constructor.
     *
     * @param name The card's title.
     */
    Card(std::string name);
};