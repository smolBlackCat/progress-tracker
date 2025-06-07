/**
 * Progress Stress Situation Creator
 *
 * This program creates three common situations for testing out the
 * application's performance.
 *
 *  * Basic usage: Normal use case that represents realistically user
 *                 expectations
 *  * Average usage: Edge use case that represents an user stressing the
 *                   Progress App.
 *  * Extreme: The most heavy possible use case. Despite the name, the quantity
 *             is not very extraordinary since it is still based on 2-digit
 *             numbers as it is very unlikely the user will have more than
 *             50 boards.
 */

#include <core/board-manager.h>

#include <chrono>
#include <string>
#include <tuple>

using namespace std::chrono_literals;

enum class StressType { USER_CASE, EDGE, EXTREME };

using StressParameters = std::tuple<short, short, short, short>;

StressParameters create_stress_parameters(StressType type) {
    switch (type) {
        case StressType::USER_CASE:
            return StressParameters(5, 10, 15, 10);
        case StressType::EDGE:
            return StressParameters(20, 20, 30, 20);
        case StressType::EXTREME:
            return StressParameters(50, 50, 50, 50);
        default:
            throw std::invalid_argument{"Possible corruption"};
    }
}

StressType create_stress_type(const std::string& type) {
    if (type == "user_case") {
        return StressType::USER_CASE;
    } else if (type == "edge") {
        return StressType::EDGE;
    } else if (type == "extreme") {
        return StressType::EXTREME;
    }
    throw std::invalid_argument("Invalid stress type");
}

// TODO: Write CLI app that generates the correct amount depending on the
// given option
int main(int argc, char* argv[]) {
    std::string dir = argv[1];
    BoardManager bm{dir};

    std::string stress_type = argv[2];
    StressParameters params;
    try {
        params = create_stress_parameters(create_stress_type(stress_type));
    } catch (std::invalid_argument& err) {
        std::cerr << err.what() << '\n';
    }

    for (short i = 0; i < std::get<0>(params); ++i) {
        const std::string cur_board_filename =
            bm.local_add(std::format("Board {}", i), "rgb(0,0,140)");

        auto board = bm.local_open(cur_board_filename);

        for (short j = 0; j < std::get<1>(params); ++j) {
            auto cardlist = board->container().append(
                CardList{std::format("CardList {}", j)});
            for (short k = 0; k < std::get<2>(params); ++k) {
                auto card = cardlist->container().append(
                    Card{std::format("Card {}", k)});
                card->set_color(Color{165, 29, 45, 1.0});
                card->set_due_date(Date(2025y, June, 5d));
                card->set_notes(
                    "Progress is supposed to be simple and I love my "
                    "girlfriend a lot");
                for (short l = 0; l < std::get<3>(params); ++l) {
                    card->container().append(Task{std::format("Task {}", l)});
                }
            }
        }

        bm.local_save(board);
        bm.local_close(board);
    }
}

