#include <iostream>
#define CATCH_CONFIG_MAIN

#include "card.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Card operations", "[Card]") {
    // Creating a dummy color for testing
    Color red{255, 0, 0, 1};  // Solid red color

    Card card("MyCard");

    SECTION("Color operations") {
        REQUIRE(card.is_color_set() == false);

        card.set_color(red);
        REQUIRE(card.get_color() == red);
        REQUIRE(card.is_color_set() == true);

        card.set_color(NO_COLOR);
        REQUIRE(card.is_color_set() == false);
    }

    SECTION("Notes operations") {
        REQUIRE(card.get_notes().empty());

        std::string notes = "This is a test note.";
        card.set_notes(notes);

        REQUIRE(card.get_notes() == notes);
    }

    SECTION("Task management") {
        Task task1("Task1");
        Task task2("Task2", true);
        Task task3("Task3");

        auto taskPtr1 = card.add_task(task1);
        auto taskPtr2 = card.add_task(task2);
        auto taskPtr3 = card.add_task(task3);

        REQUIRE(card.get_tasks().size() == 3);
        REQUIRE(*taskPtr1 == task1);
        REQUIRE(*taskPtr2 == task2);
        REQUIRE(*taskPtr3 == task3);

        REQUIRE(card.remove_task(taskPtr2) == true);
        REQUIRE(card.get_tasks().size() == 2);

        REQUIRE(card.remove_task(taskPtr2) ==
                false);  // Removing again should return false
    }

    SECTION("Task completion") {
        Task task1("Task1", true);   // Completed task
        Task task2("Task2", false);  // Incomplete task

        card.add_task(task1);
        auto task2_sptr = card.add_task(task2);

        double completion = card.get_completion();
        REQUIRE(completion ==
                50.0);  // 1 out of 2 tasks are done, so 50% completion

        task2_sptr->set_done(true);
        completion = card.get_completion();
        REQUIRE(completion ==
                100.0);  // Both tasks are now done, so 100% completion

        Task task3("Task3", false);  // New incomplete task
        card.add_task(task3);

        completion = card.get_completion();
        REQUIRE(completion ==
                66.0);  // 2 out of 3 tasks are done, so ~66% completion
    }
}

TEST_CASE("Task reordering", "[Card]") {
    Card card1{"Operating Systems"};

    Task task1{"Windows"};
    Task task2{"MacOS"};
    Task task3{"Debian"};

    card1.add_task(task1);
    card1.add_task(task2);
    card1.add_task(task3);

    auto& card1_tasks = card1.get_tasks();

    REQUIRE(task1 == *card1_tasks[0]);
    REQUIRE(task2 == *card1_tasks[1]);
    REQUIRE(task3 == *card1_tasks[2]);

    card1.reorder_task(task1, task3); // Moves "Windows" task after "Debian"

    CHECK(task1 == *card1_tasks[2]);
    CHECK(task2 == *card1_tasks[0]);
    CHECK(task3 == *card1_tasks[1]);
}