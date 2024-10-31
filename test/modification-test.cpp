#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>

#include "board.h"
#include "card.h"
#include "cardlist.h"
#include "item.h"
#include "task.h"

TEST_CASE("Item set name marks modified", "[Item]") {
    Item item("initial name");
    REQUIRE_FALSE(item.get_modified());
    item.set_name("new name");
    REQUIRE(item.get_modified());
}

TEST_CASE("Item set modified", "[Item]") {
    Item item("initial name");
    REQUIRE_FALSE(item.get_modified());
    item.set_modified(true);
    REQUIRE(item.get_modified());
    item.set_modified(false);
    REQUIRE_FALSE(item.get_modified());
}

TEST_CASE("CardList creation does not mark modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    REQUIRE_FALSE(cardlist.get_modified());
}

TEST_CASE("CardList add card marks modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    REQUIRE_FALSE(cardlist.get_modified());
    Card card("initial card", NO_COLOR);
    cardlist.add(card);
    REQUIRE(cardlist.get_modified());
}

TEST_CASE("CardList remove card marks modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    Card card("initial card", NO_COLOR);
    cardlist.add(card);
    cardlist.set_modified(false);
    REQUIRE_FALSE(cardlist.get_modified());
    cardlist.remove(card);
    REQUIRE(cardlist.get_modified());
}

TEST_CASE("CardList reorder cards marks modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    Card card1("card1", NO_COLOR);
    Card card2("card2", NO_COLOR);
    cardlist.add(card1);
    cardlist.add(card2);
    cardlist.set_modified(false);
    REQUIRE_FALSE(cardlist.get_modified());
    cardlist.reorder(card1, card2);
    REQUIRE(cardlist.get_modified());
}

TEST_CASE("CardList modify card marks cardlist modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    Card card("initial card", NO_COLOR);
    cardlist.add(card);
    cardlist.set_modified(false);
    REQUIRE_FALSE(cardlist.get_modified());
    cardlist.get_cards().at(0)->set_name("new card name");
    REQUIRE(cardlist.get_modified());
}

TEST_CASE("Card creation does not mark modified", "[Card]") {
    Card card("initial card", NO_COLOR);
    REQUIRE_FALSE(card.get_modified());
}

TEST_CASE("Card add task marks modified", "[Card]") {
    Card card("initial card", NO_COLOR);
    REQUIRE_FALSE(card.get_modified());
    Task task("initial task");
    card.add(task);
    REQUIRE(card.get_modified());
}

TEST_CASE("Card set name marks modified", "[Card]") {
    Card card("initial card", NO_COLOR);
    REQUIRE_FALSE(card.get_modified());
    card.set_name("new card name");
    REQUIRE(card.get_modified());
}

TEST_CASE("Card set notes marks modified", "[Card]") {
    Card card("initial card", NO_COLOR);
    REQUIRE_FALSE(card.get_modified());
    card.set_notes("new notes");
    REQUIRE(card.get_modified());
}

TEST_CASE("Card modify task marks card modified", "[Card]") {
    Card card("initial card", NO_COLOR);
    Task task("initial task");
    card.add(task);
    card.set_modified(false);
    REQUIRE_FALSE(card.get_modified());
    card.get_tasks().at(0)->set_done(true);
    REQUIRE(card.get_modified());
}

TEST_CASE("Task creation does not mark modified", "[Task]") {
    Task task("initial task");
    REQUIRE_FALSE(task.get_modified());
}

TEST_CASE("Task set done marks modified", "[Task]") {
    Task task("initial task");
    REQUIRE_FALSE(task.get_modified());
    task.set_done(true);
    REQUIRE(task.get_modified());
}

TEST_CASE("Task set name marks modified", "[Task]") {
    Task task("initial task");
    REQUIRE_FALSE(task.get_modified());
    task.set_name("new task name");
    REQUIRE(task.get_modified());
}
