#define CATCH_CONFIG_MAIN

#include <core/colorable.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Converting RGBA to HEX", "[rgb_to_hex]") {
    SECTION("Opaque Black") {
        Color color{0, 0, 0, 1};
        CHECK(rgb_to_hex(color) == 0x000000FF);
    }

    SECTION("Transparent Black") {
        Color color{0, 0, 0, 0};
        CHECK(rgb_to_hex(color) == 0x00000000);
    }

    SECTION("Transparent White") {
        Color color{255, 255, 255, 0};
        CHECK(rgb_to_hex(color) == 0xFFFFFF00);
    }

    SECTION("Opaque White") {
        Color color{255, 255, 255, 1};
        CHECK(rgb_to_hex(color) == 0xFFFFFFFF);
    }

    SECTION("Custom colour") {
        Color color{19, 22, 24, 1};
        CHECK(rgb_to_hex(color) == 0x131618FF);
    }
}

TEST_CASE("Creating Color from std::string", "[string_to_color]") {
    SECTION("RGBA color code: Variant alpha") {
        std::string rgba_color = "rgba(12,45,86,0.7)";
        Color c = string_to_color(rgba_color);

        CHECK(std::get<0>(c) == 12);
        CHECK(std::get<1>(c) == 45);
        CHECK(std::get<2>(c) == 86);
        CHECK(std::get<3>(c) == 0.7f);
    }

    SECTION("RGBA color code: Solid") {
        std::string rgba_color = "rgba(12,45,86,1)";
        Color c = string_to_color(rgba_color);

        CHECK(std::get<0>(c) == 12);
        CHECK(std::get<1>(c) == 45);
        CHECK(std::get<2>(c) == 86);
        CHECK(std::get<3>(c) == 1.0f);
    }

    SECTION("RGBA color code") {
        std::string rgba_color = "rgb(12,45,86)";
        Color c = string_to_color(rgba_color);

        CHECK(std::get<0>(c) == 12);
        CHECK(std::get<1>(c) == 45);
        CHECK(std::get<2>(c) == 86);
    }

    SECTION("Invalid color code") {
        CHECK(string_to_color("not-a-color") == NO_COLOR);
    }
}