#include <tinyxml2.h>

#include <filesystem>
#include <format>
#include <iostream>
#include <sstream>

#include "core/board.h"

/**
 * This small script generates a very heavy board. This is used to stress
 * Progress
 */
int main() {
    tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument{};

    auto board_element = doc->NewElement("board");
    board_element->SetAttribute("name", "Pretty Heavy Board");
    board_element->SetAttribute("background", "rgba(255,0,0,1)");

    for (int i = 0; i < 50; i++) {
        auto cur_list_element = board_element->InsertNewChildElement("list");
        cur_list_element->SetAttribute("name",
                                       std::format("Cardlist {}", i).c_str());

        for (int j = 0; j < 50; j++) {
            auto cur_card_element =
                cur_list_element->InsertNewChildElement("card");
            cur_card_element->SetAttribute("name",
                                           std::format("Card {}", j).c_str());
            cur_card_element->SetAttribute("color", "rgb(0,0,150)");
            for (int k = 0; k < 50; k++) {
                auto cur_task_element =
                    cur_card_element->InsertNewChildElement("task");

                cur_task_element->SetAttribute(
                    "name", std::format("Task {}", k).c_str());
                cur_task_element->SetAttribute("done", false);
            }
        }
    }

    doc->InsertEndChild(board_element);

    doc->SaveFile("heavy-board.xml");
    auto filepath = "heavy-board.xml";
    BoardBackend backend{BackendType::LOCAL, std::map<std::string, std::string>{
                                                 {"filepath", filepath}}};
    Board board = backend.load();
    std::string ss;
    std::cin >> ss;
    delete doc;
}