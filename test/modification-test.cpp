#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

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

TEST_CASE("Board creation does not mark modified", "[Board]") {
    Board board("initial board", "rgba(0,0,0,1)");
    REQUIRE_FALSE(board.get_modified());
}

TEST_CASE("Board add cardlist marks modified", "[Board]") {
    Board board("initial board", "rgba(0,0,0,1)");
    REQUIRE_FALSE(board.get_modified());
    CardList cardlist("initial cardlist");
    board.add_cardlist(cardlist);
    REQUIRE(board.get_modified());
}

TEST_CASE("Board remove cardlist marks modified", "[Board]") {
    Board board("initial board", "rgba(0,0,0,1)");
    CardList cardlist("initial cardlist");
    board.add_cardlist(cardlist);
    board.set_modified(false);
    REQUIRE_FALSE(board.get_modified());
    board.remove_cardlist(cardlist);
    REQUIRE(board.get_modified());
}

TEST_CASE("Board reorder cardlists marks modified", "[Board]") {
    Board board("initial board", "rgba(0,0,0,1)");
    CardList cardlist1("cardlist1");
    CardList cardlist2("cardlist2");
    board.add_cardlist(cardlist1);
    board.add_cardlist(cardlist2);
    board.set_modified(false);
    REQUIRE_FALSE(board.get_modified());
    board.reorder_cardlist(board.get_cardlist_vector()[0],
                           board.get_cardlist_vector()[1]);
    REQUIRE(board.get_modified());
}

TEST_CASE("Board modify cardlist marks board modified", "[Board]") {
    Board board("initial board", "rgba(0,0,0,1)");
    CardList cardlist("initial cardlist");
    board.add_cardlist(cardlist);
    board.set_modified(false);
    REQUIRE_FALSE(board.get_modified());
    board.get_cardlist_vector()[0]->set_name("new cardlist name");
    REQUIRE(board.get_modified());
}

TEST_CASE("Board modify card marks board modified", "[Board]") {
    Board board("initial board", "rgba(0,0,0,1)");
    CardList cardlist("initial cardlist");
    Card card("initial card", NO_COLOR);
    cardlist.add_card(card);
    board.add_cardlist(cardlist);
    board.set_modified(false);
    REQUIRE_FALSE(board.get_modified());
    board.get_cardlist_vector()[0]->get_card_vector()[0]->set_name(
        "new card name");
    REQUIRE(board.get_modified());
}

TEST_CASE("Board modify task marks board modified", "[Board]") {
    Board board("initial board", "rgba(0,0,0,1)");
    CardList cardlist("initial cardlist");
    Card card("initial card", NO_COLOR);
    Task task("initial task");
    card.add_task(task);
    cardlist.add_card(card);
    board.add_cardlist(cardlist);
    board.set_modified(false);
    REQUIRE_FALSE(board.get_modified());
    board.get_cardlist_vector()[0]
        ->get_card_vector()[0]
        ->get_tasks()
        .at(0)
        ->set_done(true);
    REQUIRE(board.get_modified());
}

TEST_CASE("CardList creation does not mark modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    REQUIRE_FALSE(cardlist.get_modified());
}

TEST_CASE("CardList add card marks modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    REQUIRE_FALSE(cardlist.get_modified());
    Card card("initial card", NO_COLOR);
    cardlist.add_card(card);
    REQUIRE(cardlist.get_modified());
}

TEST_CASE("CardList remove card marks modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    Card card("initial card", NO_COLOR);
    cardlist.add_card(card);
    cardlist.set_modified(false);
    REQUIRE_FALSE(cardlist.get_modified());
    cardlist.remove_card(card);
    REQUIRE(cardlist.get_modified());
}

TEST_CASE("CardList reorder cards marks modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    Card card1("card1", NO_COLOR);
    Card card2("card2", NO_COLOR);
    auto scard1 = cardlist.add_card(card1);
    auto scard2 = cardlist.add_card(card2);
    cardlist.set_modified(false);
    REQUIRE_FALSE(cardlist.get_modified());
    cardlist.reorder_card(scard1, scard2);
    REQUIRE(cardlist.get_modified());
}

TEST_CASE("CardList modify card marks cardlist modified", "[CardList]") {
    CardList cardlist("initial cardlist");
    Card card("initial card", NO_COLOR);
    cardlist.add_card(card);
    cardlist.set_modified(false);
    REQUIRE_FALSE(cardlist.get_modified());
    cardlist.get_card_vector().at(0)->set_name("new card name");
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
    card.add_task(task);
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
    card.add_task(task);
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
