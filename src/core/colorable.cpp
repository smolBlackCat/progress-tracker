#include "colorable.h"

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
    std::regex color_code_r{"\\d{1,3}"};

    auto color_code_begin =
        std::sregex_iterator(str.begin(), str.end(), color_code_r);
    auto color_code_end = std::sregex_iterator();

    if (std::distance(color_code_begin, color_code_end) == 3) {
        uint8_t r = std::stoi(std::smatch{*color_code_begin}.str());
        uint8_t g =
            std::stoi(std::smatch{*(std::next(color_code_begin, 1))}.str());
        uint8_t b =
            std::stoi(std::smatch{*(std::next(color_code_begin, 2))}.str());
        return {r, g, b, 1.0};
    } else {
        return NO_COLOR;
    }
}