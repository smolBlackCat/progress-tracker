#pragma once

#include <cstdint>
#include <string>
#include <tuple>

typedef std::tuple<unsigned char, unsigned char, unsigned char, float> Color;

static const Color NO_COLOR(0, 0, 0, 0.0);

/**
 * @brief Describes items that may have colours
 */
class Colorable {
public:
    Colorable() = default;
    virtual ~Colorable() = default;

    /**
     * @brief Sets a colour to an item
     */
    virtual void set_color(const Color& rgb) = 0;

    /**
     * @brief Returns the colour of an item
     */
    virtual Color get_color() const { return color; }

    /**
     * @brief Indicates whether a colour was set to an item
     *
     * Essentially, a colour is set when it's alpha is not 0
     */
    virtual bool is_color_set() const { return color != NO_COLOR; }

protected:
    Color color;
};

/**
 * @brief Translates a color data into a string in the format rgb(x,y,z)
 *
 * @param color color data
 */
std::string color_to_string(const Color& color);

/**
 * @brief Transform color data into a number
 *
 * @param color color data
 */
uint32_t rgb_to_hex(const Color& color);

/**
 * @brief Translates a string in the form rgb(x,y,z) into a color data
 *
 * @param str string representing a color
 */
Color string_to_color(const std::string& str);