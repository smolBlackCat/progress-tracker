#include <board-card-button.h>
#include <board.h>
#include <gtkmm.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

class TestWindow : public Gtk::Window {
public:
    TestWindow() : Gtk::Window{}, root{}, colour{"fatal"} {
        // Load boards
        std::string board_path =
            std::string{std::getenv("HOME")} + "/.config/progress/boards/";
        int col = 0;
        for (const auto& dir_entry :
             std::filesystem::directory_iterator(board_path)) {
            Board* p_board = board_from_xml(dir_entry.path());
            boards.push_back(p_board);
            auto board = Gtk::make_managed<ui::BoardCardButton>(*p_board);
            board->signal_clicked().connect([this]() {
                std::cout << "Fatal" << colour.to_string() << std::endl;
            });
            root.append(*board);
        }
        root.set_margin(20);

        // Probably the code will break
        set_child(root);
    }

    ~TestWindow() {
        for (auto& board : boards) {
            delete board;
        }
    }

    void fatal() { std::cout << "Fatal" << colour.to_string() << std::endl; }

private:
    Gtk::Box root;
    Gdk::RGBA colour;
    std::vector<Board*> boards;
};

int main(int argv, char* argc[]) {
    auto app = Gtk::Application::create("board-card-test");

    return app->make_window_and_run<TestWindow>(argv, argc);
}