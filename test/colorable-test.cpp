#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>
#include <core/colorable.h>

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