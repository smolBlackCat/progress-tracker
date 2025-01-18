#include "colorable.h"

#include <sstream>

std::string color_to_string(const Color& color) {
    std::ostringstream oss{};
    oss << "rgb(" << std::to_string(std::get<0>(color)) << ","
        << std::to_string(std::get<1>(color)) << ","
        << std::to_string(std::get<2>(color)) << ")";

    return oss.str();
}

uint32_t rgb_to_hex(const Color& color) {
    uint8_t r = std::get<0>(color);
    uint8_t g = std::get<1>(color);
    uint8_t b = std::get<2>(color);
    uint8_t a = (uint8_t)std::get<3>(color) * 255;

    return (r << 24) | (g << 16) | (b << 8) | a;
}

Color string_to_color(const std::string& str) {
    // Check the basic format
    if (str.size() < 10 || str.substr(0, 4) != "rgb(" || str.back() != ')') {
        return NO_COLOR;
    }

    // Extract the numbers as substrings
    size_t start = 4;  // Skip "rgb("
    size_t end = str.find(',', start);
    if (end == std::string::npos) return NO_COLOR;
    int r = std::stoi(str.substr(start, end - start));

    start = end + 1;
    end = str.find(',', start);
    if (end == std::string::npos) return NO_COLOR;
    int g = std::stoi(str.substr(start, end - start));

    start = end + 1;
    end = str.find(')', start);
    if (end == std::string::npos) return NO_COLOR;
    int b = std::stoi(str.substr(start, end - start));

    // Ensure values are in the 0-255 range
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        return NO_COLOR;
    }

    return {static_cast<uint8_t>(r), static_cast<uint8_t>(g),
            static_cast<uint8_t>(b), 1.0f};
}