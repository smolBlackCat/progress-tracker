#include <core/board-manager.h>

#include <iostream>

int main() {
    BoardManager board_manager;

    // board_manager.local_add("Computer Science", "rgba(1,1,1,1)");

    // Iterating through the board manager
    for (const auto& board_entry : board_manager.local_boards()) {
        std::cout << "* " << board_entry.filename << '\n';
    }

    std::string filename = board_manager.local_boards()[0].filename;
    std::shared_ptr<Board> board1 = board_manager.local_open(filename);

    auto cardlist = board1->container().get_data()[0];
    cardlist->set_name("This is something else");
    std::cout << cardlist->modified() << '\n';
}