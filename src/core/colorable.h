#pragma once

#include <cstdint>
#include <string>
#include <tuple>

typedef std::tuple<unsigned char, unsigned char, unsigned char, float> Color;

static const Color NO_COLOR(0, 0, 0, 0.0);

const static Color RED_COLOR    = Color{165, 29,  45,  1};
const static Color ORANGE_COLOR = Color{198, 70,  0,   1};
const static Color YELLOW_COLOR = Color{229, 165, 10,  1};
const static Color GREEN_COLOR  = Color{38,  162, 105, 1};
const static Color BLUE_COLOR   = Color{26,  95,  180, 1};
const static Color PURPLE_COLOR = Color{32,  9,   65,  1};

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