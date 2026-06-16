#define CATCH_CONFIG_MAIN

#include <core/card.h>
#include <core/item-container.h>

#include <catch2/catch_test_macros.hpp>

using MockContainer = ItemContainer<Card>;

TEST_CASE("ItemContainer: Modification and Basic Growth", "[ItemContainer]") {
    MockContainer container;
    auto item1 = Card::create("New Card");

    SECTION("Initial state is unmodified") {
        CHECK_FALSE(container.modified());
        CHECK(container.size() == 0);
    }

    SECTION("Appending updates size and modification state") {
        container.append(item1);
        CHECK(container.size() == 1);
        CHECK(container.modified());
    }

    SECTION("Manual modification reset") {
        container.append(item1);
        container.modify(false);
        CHECK_FALSE(container.modified());
    }

    SECTION("Removing an item updates state") {
        container.append(item1);
        container.modify(false);  // Reset to test if remove triggers it again

        container.remove(item1);
        CHECK(container.size() == 0);
        CHECK(container.modified());
    }
}

TEST_CASE("ItemContainer: Positional Insertion", "[ItemContainer]") {
    MockContainer container;
    auto item1 = Card::create("Card 1");
    auto item2 = Card::create("Card 2");
    auto item3 = Card::create("Card 3");

    container.append(item1);
    container.append(item2);

    SECTION("Insert after a specific sibling") {
        container.insert_after(item3,
                               item1);  // Expected: [item1, item3, item2]
        auto data = container.get_data();

        REQUIRE(container.size() == 3);
        CHECK(data[1] == item3);
        CHECK(container.modified());
    }

    SECTION("Insert before a specific sibling") {
        container.insert_before(item3,
                                item2);  // Expected: [item1, item3, item2]
        auto data = container.get_data();

        REQUIRE(container.size() == 3);
        CHECK(data[1] == item3);
        CHECK(container.modified());
    }

    SECTION("Insert before/after a non-present sibling") {
        auto foreign_item = Card::create("I am foreign");
        ssize_t bf_size = container.size();

        // This should not modify
        container.insert_after(item3, foreign_item);
    }

    SECTION("Insert repeated item") {
        ssize_t bf_size = container.size();
        container.insert_after(item1, item2);

        CHECK(container.size() == bf_size);
    }
}

TEST_CASE("ItemContainer: Signal Emissions", "[ItemContainer]") {
    MockContainer container;
    auto item = Card::create("New Card");
    bool append_fired = false;
    bool remove_fired = false;
    ReorderingType last_reorder = ReorderingType::INVALID;

    // Connect stubs to signals
    container.signal_append().connect([&](auto i) { append_fired = true; });
    container.signal_remove().connect([&](auto i) { remove_fired = true; });
    container.signal_reorder().connect(
        [&](auto next, auto sib, ReorderingType type) { last_reorder = type; });

    SECTION("Append signal fires") {
        container.append(item);
        CHECK(append_fired);
    }

    SECTION("Remove signal fires") {
        container.append(item);
        container.remove(item);
        CHECK(remove_fired);
    }

    SECTION("Reorder signal carries correct ReorderingType") {
        auto item2 = Card::create("New Card");
        container.append(item);
        container.append(item2);

        container.reorder_after(item, item2);
        CHECK(last_reorder == ReorderingType::AFTER);

        container.reorder_before(item, item2);
        CHECK(last_reorder == ReorderingType::BEFORE);
    }
}

TEST_CASE("ItemContainer: Reordering", "[ItemContainer]") {
    MockContainer container;
    auto item1 = Card::create("New Card 1");
    auto item2 = Card::create("New Card 2");

    container.append(item1);
    container.append(item2);  // Current: [item1, item2]

    SECTION("Reorder item after another") {
        container.reorder_after(item1, item2);  // Expected: [item2, item1]
        auto data = container.get_data();
        CHECK(data[0] == item2);
        CHECK(data[1] == item1);
    }

    SECTION("Reorder item before another") {
        container.reorder_before(item2, item1);  // Expected: [item2, item1]
        auto data = container.get_data();
        CHECK(data[0] == item2);
    }
}