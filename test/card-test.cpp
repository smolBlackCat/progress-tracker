#define CATCH_CONFIG_MAIN

#include <core/card.h>

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

        auto taskPtr1 = card.add(task1);
        auto taskPtr2 = card.add(task2);
        auto taskPtr3 = card.add(task3);

        REQUIRE(!card.add(task1));

        REQUIRE(card.get_tasks().size() == 3);
        REQUIRE(*taskPtr1 == task1);
        REQUIRE(*taskPtr2 == task2);
        REQUIRE(*taskPtr3 == task3);

        REQUIRE(card.remove(task2) == true);
        REQUIRE(card.get_tasks().size() == 2);

        REQUIRE(card.remove(task2) ==
                false);  // Removing again should return false
    }

    SECTION("Task completion") {
        Task task1("Task1", true);   // Completed task
        Task task2("Task2", false);  // Incomplete task

        card.add(task1);
        auto task2_sptr = card.add(task2);

        double completion = card.get_completion();
        REQUIRE(completion ==
                50.0);  // 1 out of 2 tasks are done, so 50% completion

        task2_sptr->set_done(true);
        completion = card.get_completion();
        REQUIRE(completion ==
                100.0);  // Both tasks are now done, so 100% completion

        Task task3("Task3", false);  // New incomplete task
        card.add(task3);

        completion = card.get_completion();
        REQUIRE(int(completion) ==
                66);  // 2 out of 3 tasks are done, so ~66% completion
    }

    SECTION("Setting Due dates to Card objects") {
        Card card_with_due_date{"Uni assignment"};
        Date date_now = std::chrono::floor<std::chrono::days>(
            std::chrono::system_clock::now());

        Date past_due_date = date_now - std::chrono::months(2);
        Date n_past_due_date =
            std::chrono::sys_days(past_due_date + std::chrono::months{0});

        card_with_due_date.set_due_date(
            n_past_due_date);  // Regardless of today's date, this card is
                               // always past the due date

        CHECK(card_with_due_date.get_modified());
        REQUIRE(card_with_due_date.past_due_date());

        Date in_time_due_date = date_now + std::chrono::months(5);
        card_with_due_date.set_due_date(in_time_due_date);
        REQUIRE(!card_with_due_date.past_due_date());

        card_with_due_date.set_due_date(Date{});
        // When the date is invalid it simply
        // means that no date was set
        REQUIRE(!card_with_due_date.past_due_date());
    }

    SECTION("Marking Cards as complete only if a Card has a due date") {
        Card rouxls_kard{"Worms"};

        REQUIRE(rouxls_kard
                    .get_complete());  // Because no date was set, assume this
                                       // card is in an already complete state

        Date now_date = std::chrono::floor<std::chrono::days>(
            std::chrono::system_clock::now());
        rouxls_kard.set_due_date(now_date);
        REQUIRE(rouxls_kard.get_modified());

        rouxls_kard.set_modified(false);

        // Since a due date was set, it means that card expects to be completed
        // before the due date arrives
        REQUIRE(!rouxls_kard.get_complete());
        REQUIRE(!rouxls_kard.get_modified());
        rouxls_kard.set_complete(true);
        REQUIRE(rouxls_kard.get_modified());
        REQUIRE(rouxls_kard.get_complete());
    }
}

TEST_CASE("Task reordering", "[Card]") {
    Card card1{"Operating Systems"};

    Task task1{"Windows"};
    Task task2{"MacOS"};
    Task task3{"Debian"};

    card1.add(task1);
    card1.add(task2);
    card1.add(task3);

    card1.set_modified(false);

    auto& card1_tasks = card1.get_tasks();

    REQUIRE(task1 == *card1_tasks[0]);
    REQUIRE(task2 == *card1_tasks[1]);
    REQUIRE(task3 == *card1_tasks[2]);

    card1.reorder(task1, task3);  // Moves "Windows" task after "Debian"

    CHECK(task1 == *card1_tasks[2]);
    CHECK(task2 == *card1_tasks[0]);
    CHECK(task3 == *card1_tasks[1]);

    REQUIRE(card1.get_modified());
}