#define CATCH_CONFIG_MAIN
#include <core/board.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Board Instatiation", "[Board]") {
    Board board{"Name", "rgba(12,12,12,0.7)"};

    CHECK(board.get_name() == "Name");
    CHECK(board.get_background() == "rgba(12,12,12,0.700000)");
}

TEST_CASE("Name Changing", "[Board]") {
    Board target_board{"GTK Programming", "rgb(12,12,12)"};

    REQUIRE(target_board.get_name() == "GTK Programming");

    SECTION("Valid Name") {
        target_board.set_name("C++ Projects");
        REQUIRE(target_board.get_name() == "C++ Projects");
    }

    SECTION("Invalid Name") {
        target_board.set_name("");
        REQUIRE(target_board.get_name() == "GTK Programming");
    }
}

TEST_CASE("Colour Setting", "[Board]") {
    Board target_board{"Custom Board", "rgb(12,12,12)"};

    SECTION("Solid Colour") {
        target_board.set_background(Color{255, 255, 255, 1.0});

        REQUIRE(target_board.get_background() == "rgb(255,255,255)");
    }

    SECTION("Slightly Transparent Colour") {
        target_board.set_background(Color{122, 12, 23, 0.5});

        CHECK(target_board.get_background() == "rgba(122,12,23,0.500000)");
    }

    SECTION("Invalid filename") {
        target_board.set_background("");

        REQUIRE(target_board.get_background() == "rgb(12,12,12)");
    }
}

TEST_CASE("Signal Emission", "[Board]") {
    Board target_board{"Simple Name", "rgba(12,23,18,0.4)"};

    bool name_changed = false;
    bool background_changed = false;

    target_board.signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });
    target_board.signal_background().connect(
        [&background_changed](std::string) { background_changed = true; });

    SECTION("Name Changed Signal: Valid Name") {
        target_board.set_name("Not a simple name anymore");

        REQUIRE(name_changed);
    }

    SECTION("Name Changed Signal: Invalid Name") {
        target_board.set_name("");

        REQUIRE_FALSE(name_changed);
    }

    SECTION("Background Changed Signal: Valid Background") {
        target_board.set_background(Color{12, 12, 12, 1.0});

        REQUIRE(background_changed);
    }

    SECTION("Background Changed Signal: Invalid Background") {
        target_board.set_background("not-a-color");

        REQUIRE_FALSE(background_changed);
    }
}

TEST_CASE("Modification State", "[Board]") {
    Board target_board{"Name", "rgba(1,1,1,0.1)"};

    REQUIRE_FALSE(target_board.modified());

    SECTION("Modify after modify call") {
        target_board.modify();
        REQUIRE(target_board.modified());
    }

    SECTION("Modify after name change") {
        target_board.set_name("Pietra");
        REQUIRE(target_board.modified());
    }

    SECTION("Do not modify after invalid name change") {
        target_board.set_name("");
        REQUIRE_FALSE(target_board.modified());
    }

    SECTION("Modify after background change") {
        target_board.set_background(Color{1, 1, 1, 1});
        REQUIRE(target_board.modified());
    }

    SECTION("Do not modify after invalid background change") {
        target_board.set_background("not-a-valid-background");
        REQUIRE_FALSE(target_board.modified());
    }

    SECTION("Modify after container alteration") {
        target_board.container().append(CardList{"Something else"});
        REQUIRE(target_board.modified());
    }
}