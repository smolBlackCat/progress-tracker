#pragma once

#include "colorable.h"
#include "item.h"

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

protected:
    Gdk::RGBA color = NO_COLOR;
};