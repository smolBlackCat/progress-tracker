#define CATCH_CONFIG_MAIN

#include <core/card.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Card Instantiation", "[Card]") {
    auto card = Card{"Computer Science"};

    REQUIRE_FALSE(card.modified());

    CHECK(card.get_name() == "Computer Science");
    CHECK(card.get_notes() == "");
    CHECK(card.get_due_date() == Date{});  // means unset
    CHECK(card.get_complete() == true);    // Questionable
}

TEST_CASE("Signal Emission", "[Card]") {
    auto card = Card{"Software Engineering"};

    bool name_changed, notes_changed, color_changed, due_changed,
        complete_changed, appended, removed, reordered;
    name_changed = notes_changed = color_changed = due_changed =
        complete_changed = appended = removed = reordered = false;

    card.signal_name_changed().connect(
        [&name_changed]() { name_changed = true; });

    card.signal_color().connect(
        [&color_changed](Color, Color) { color_changed = true; });

    card.signal_notes().connect(
        [&notes_changed](std::string, std::string) { notes_changed = true; });

    card.signal_due_date().connect(
        [&due_changed](Date, Date) { due_changed = true; });

    card.signal_complete().connect(
        [&complete_changed](bool) { complete_changed = true; });

    card.container().signal_append().connect(
        [&appended](std::shared_ptr<Task>) { appended = true; });

    card.container().signal_remove().connect(
        [&removed](std::shared_ptr<Task>) { removed = true; });

    card.container().signal_reorder().connect(
        [&reordered](std::shared_ptr<Task>, std::shared_ptr<Task>,
                     ReorderingType) { reordered = true; });

    SECTION("Name Changing") {
        card.set_name("Operating Systems");

        CHECK(name_changed);
    }

    SECTION("Notes Setting") {
        card.set_notes(
            "Key feature of modern operating systems is their "
            "multitasking capability");

        CHECK(notes_changed);
    }

    SECTION("Colour Setting") {
        card.set_color(Color{32, 192, 12, 0.6});

        CHECK(color_changed);
    }

    SECTION("Due Date Setting") {
        card.set_due_date(Date(std::chrono::year(2012), std::chrono::month(12),
                               std::chrono::day(12)));
        CHECK(due_changed);
    }

    SECTION("Complete State") {
        card.set_due_date(Date(std::chrono::year(1998), std::chrono::month(9),
                               std::chrono::day(7)));
        card.set_complete(true);

        CHECK(complete_changed);
    }

    SECTION("Complete State: No due time set") {
        card.set_complete(true);

        CHECK_FALSE(complete_changed);
    }

    SECTION("Appending Tasks") {
        card.container().append(Task{"nobody here"});

        CHECK(appended);
    }

    SECTION("Appending Repeating Tasks") {
        Task task{"I should not be added twice because of my ID"};
        card.container().append(task);
        card.container().modify(false);
        appended = false;

        card.container().append(task);
        CHECK_FALSE(appended);
    }

    SECTION("Removing Tasks") {
        Task task1{"Cleanup home system"};
        Task task2{"Remove dirt from the tubes"};

        card.container().append(task1);
        card.container().append(task2);
        card.container().modify(false);

        card.container().remove(task1);

        CHECK(removed);
    }

    SECTION("Removing Non-Existing Tasks") {
        Task task1{"Cleanup home system"};
        Task task2{"Remove dirt from the tubes"};

        card.container().append(task1);
        card.container().append(task2);
        card.container().modify(false);

        card.container().remove(Task{"I don't belong here"});

        CHECK_FALSE(removed);
    }

    SECTION("Reordering Tasks") {
        Task task1{"Cleanup home system"};
        Task task2{"Remove dirt from the tubes"};

        card.container().append(task1);
        card.container().append(task2);
        card.container().modify(false);

        card.container().reorder(task1, task2);

        CHECK(reordered);
    }

    SECTION("Reordering Non-Existing Tasks") {
        Task task1{"Cleanup home system"};
        Task task2{"Remove dirt from the tubes"};

        card.container().append(task1);
        card.container().append(task2);
        card.container().modify(false);

        card.container().reorder(task1, Task{"I should not be here"});

        CHECK_FALSE(reordered);
    }

    SECTION("Inserting items at beginning") {
        const int n_tasks = 5;
        std::array<std::shared_ptr<Task>, n_tasks> ptrs;
        for (int i = 0; i < n_tasks; i++) {
            ptrs[i] = card.container().append(Task{std::format("Task {}", i)});
        }

        REQUIRE(card.container().get_data().size() == 5);
        REQUIRE(card.container().modified());

        Task task = Task{"New Card"};
        card.container().insert_before(task, *ptrs[0]);

        CHECK((card.container().get_data()[0]->get_name()) == task.get_name());
    }

    SECTION("Inserting items at the end") {
        const int n_tasks = 5;
        std::array<std::shared_ptr<Task>, n_tasks> ptrs;
        for (int i = 0; i < n_tasks; i++) {
            ptrs[i] = card.container().append(Task{std::format("Card {}", i)});
        }

        REQUIRE(card.container().get_data().size() == 5);
        REQUIRE(card.container().modified());

        Task task = Task{"New Card"};
        card.container().insert_before(task, *ptrs[n_tasks - 1]);

        CHECK((card.container().get_data()[n_tasks - 1])->get_name() ==
              task.get_name());
    }

    SECTION("Inserting items at middle") {
        const int n_tasks = 5;
        std::array<std::shared_ptr<Task>, n_tasks> ptrs;
        for (int i = 0; i < n_tasks; i++) {
            ptrs[i] = card.container().append(Task{std::format("Task {}", i)});
        }

        REQUIRE(card.container().get_data().size() == 5);
        REQUIRE(card.container().modified());

        Task task = Task{"New Task"};
        // We're roughly adding it in the middle;
        card.container().insert_before(task, *ptrs[(n_tasks + 1) / 2]);

        CHECK(*(card.container().get_data()[(n_tasks + 1) / 2]) == task);

        Task another_task = Task{"Another one"};
        // We're roughly adding it in the middle;
        card.container().insert_after(another_task, *ptrs[((n_tasks + 1) / 2)]);
        CHECK((card.container().get_data()[((n_tasks + 1) / 2) + 2])
                  ->get_name() == another_task.get_name());
    }
}

TEST_CASE("Modification State", "[Card]") {
    auto card = Card{"Software Engineering"};

    SECTION("Name Changing") {
        card.set_name("Operating Systems");

        CHECK(card.modified());
    }

    SECTION("Notes Setting") {
        card.set_notes(
            "Key feature of modern operating systems is their "
            "multitasking capability");

        CHECK(card.modified());
    }

    SECTION("Colour Setting") {
        card.set_color(Color{32, 192, 12, 0.6});

        CHECK(card.modified());
    }

    SECTION("Due Date Setting") {
        card.set_due_date(Date(std::chrono::year(2012), std::chrono::month(12),
                               std::chrono::day(12)));
        CHECK(card.modified());
    }

    SECTION("Complete State") {
        card.set_due_date(Date(std::chrono::year(1998), std::chrono::month(9),
                               std::chrono::day(7)));
        card.set_complete(true);

        CHECK(card.modified());
    }

    SECTION("Complete State: No due time set") {
        card.set_complete(true);

        CHECK_FALSE(card.modified());
    }

    SECTION("Appending Tasks") {
        card.container().append(Task{"nobody here"});

        CHECK(card.modified());
    }

    SECTION("Appending Repeating Tasks") {
        Task task{"I should not be added twice because of my ID"};
        card.container().append(task);
        card.container().modify(false);

        card.container().append(task);
        CHECK_FALSE(card.modified());
    }

    SECTION("Removing Tasks") {
        Task task1{"Cleanup home system"};
        Task task2{"Remove dirt from the tubes"};

        card.container().append(task1);
        card.container().append(task2);
        card.container().modify(false);

        card.container().remove(task1);

        CHECK(card.modified());
    }

    SECTION("Removing Non-Existing Tasks") {
        Task task1{"Cleanup home system"};
        Task task2{"Remove dirt from the tubes"};

        card.container().append(task1);
        card.container().append(task2);
        card.container().modify(false);

        card.container().remove(Task{"I don't belong here"});

        CHECK_FALSE(card.modified());
    }

    SECTION("Reordering Tasks") {
        Task task1{"Cleanup home system"};
        Task task2{"Remove dirt from the tubes"};

        card.container().append(task1);
        card.container().append(task2);
        card.container().modify(false);

        card.container().reorder(task1, task2);

        CHECK(card.modified());
    }

    SECTION("Reordering Non-Existing Tasks") {
        Task task1{"Cleanup home system"};
        Task task2{"Remove dirt from the tubes"};

        card.container().append(task1);
        card.container().append(task2);
        card.container().modify(false);

        card.container().reorder(task1, Task{"I should not be here"});

        CHECK_FALSE(card.modified());
    }
}

