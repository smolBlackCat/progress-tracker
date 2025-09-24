#include <core/board-manager.h>

#include <array>
#include <chrono>
#include <iostream>

namespace cr = std::chrono;

int main() {
    constexpr size_t SAMPLE_SIZE = 100000;
    BoardManager board_manager{"test/boards/"};
    std::array<std::shared_ptr<Board>, SAMPLE_SIZE> boards{};

    auto add_now = cr::system_clock::now();
    for (size_t i = 0; i < boards.size(); i++) {
        boards[i] = board_manager.local_open(board_manager.local_add(
            std::format("Board {}", i), "rgba(1,1,1,1)"));
    }
    auto add_end = cr::system_clock::now();
    std::cout << std::format(
        "Adding {} boards time: {}ms\n", SAMPLE_SIZE,
        cr::duration_cast<cr::milliseconds>(add_end - add_now).count());

    auto now = cr::system_clock::now();
    board_manager.local_remove(boards[SAMPLE_SIZE - 1]);
    auto end = cr::system_clock::now();
    std::cout << std::format(
        "Removing the last board item time: {}ms\n",
        cr::duration_cast<cr::milliseconds>(end - now).count());

    // Cleaning
    for (const auto& b : boards) {
        board_manager.local_remove(b);
    }
}