#pragma once

#include <gdkmm/rgba.h>

static const Gdk::RGBA NO_COLOR{0, 0, 0, 0};

/**
 * @brief Describes items that may have colours
 */
class Colorable {
public:
    virtual ~Colorable() = default;

    /**
     * @brief Sets a colour to an item
     */
    virtual void set_color(const Gdk::RGBA& color)=0;

    /**
     * @brief Returns the colour of an item
     */
    virtual Gdk::RGBA get_color() const=0;

    /**
     * @brief Indicates whether a colour was set to an item
     */
    virtual bool is_color_set()=0;
};
