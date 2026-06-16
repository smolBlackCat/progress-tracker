#define CATCH_CONFIG_MAIN

#include <core/card.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Card Instantiation", "[Card]") {
    auto card = Card::create("Computer Science");

    REQUIRE_FALSE(card->modified());

    CHECK(card->get_name() == "Computer Science");
    CHECK(card->get_notes() == "");
    CHECK(card->get_due_date() == Date{});  // means unset
    CHECK(card->get_complete() == true);    // Questionable
}

TEST_CASE("Signal Emission", "[Card]") {
    auto card = Card::create("Software Engineering");

    bool name_changed, notes_changed, color_changed, due_changed,
        complete_changed, appended, removed, reordered;

    name_changed = notes_changed = color_changed = due_changed =
        complete_changed = false;

    card->signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });

    card->signal_color().connect(
        [&color_changed](Color, Color) { color_changed = true; });

    card->signal_notes().connect(
        [&notes_changed](std::string, std::string) { notes_changed = true; });

    card->signal_due_date().connect(
        [&due_changed](Date, Date) { due_changed = true; });

    card->signal_complete().connect(
        [&complete_changed](bool) { complete_changed = true; });

    SECTION("Name Changing") {
        card->set_name("Operating Systems");

        CHECK(name_changed);
    }

    SECTION("Notes Setting") {
        card->set_notes(
            "Key feature of modern operating systems is their "
            "multitasking capability");

        CHECK(notes_changed);
    }

    SECTION("Colour Setting") {
        card->set_color(Color{32, 192, 12, 0.6});

        CHECK(color_changed);
    }

    SECTION("Due Date Setting") {
        card->set_due_date(Date(std::chrono::year(2012), std::chrono::month(12),
                                std::chrono::day(12)));
        CHECK(due_changed);
    }

    SECTION("Complete State") {
        card->set_due_date(Date(std::chrono::year(1998), std::chrono::month(9),
                                std::chrono::day(7)));
        card->set_complete(true);

        CHECK(complete_changed);
    }

    SECTION("Complete State: No due time set") {
        card->set_complete(true);

        CHECK_FALSE(complete_changed);
    }
}

TEST_CASE("Modification State", "[Card]") {
    auto card = Card::create("Software Engineering");

    SECTION("Name Changing") {
        card->set_name("Operating Systems");

        CHECK(card->modified());
    }

    SECTION("Notes Setting") {
        card->set_notes(
            "Key feature of modern operating systems is their "
            "multitasking capability");

        CHECK(card->modified());
    }

    SECTION("Colour Setting") {
        card->set_color(Color{32, 192, 12, 0.6});

        CHECK(card->modified());
    }

    SECTION("Due Date Setting") {
        card->set_due_date(Date(std::chrono::year(2012), std::chrono::month(12),
                                std::chrono::day(12)));
        CHECK(card->modified());
    }

    SECTION("Complete State") {
        card->set_due_date(Date(std::chrono::year(1998), std::chrono::month(9),
                                std::chrono::day(7)));
        card->set_complete(true);

        CHECK(card->modified());
    }

    SECTION("Complete State: No due time set") {
        card->set_complete(true);

        CHECK_FALSE(card->modified());
    }
}
