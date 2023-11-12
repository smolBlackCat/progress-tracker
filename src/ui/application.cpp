#include <cstdlib>
#include <filesystem>
#include <iostream>
#include "application.h"

Glib::RefPtr<ui::Application> ui::Application::create() {
    return Glib::RefPtr<ui::Application>(new Application());
}

ui::Application::Application() : Gtk::Application{"com.moura.Progress"},
                                 main_window{},
                                 boards{} { }

ui::Application::~Application() {
    for (auto &board : boards) {
        delete board;
        board = nullptr;
    }
}

void ui::Application::on_startup() {

    Gtk::Application::on_startup();
    std::string app_dir{std::getenv("HOME")};
    app_dir += + "/.config/progress/boards";

    if (!std::filesystem::exists(app_dir)) {
        if (!std::filesystem::create_directories(app_dir)) {
            std::cerr << "\033[;32mIt was not possible to create a directory\033[m" << std::endl;
        }
    } else {
        // Load all boards information onto memory
        for (const auto &dir_entry : std::filesystem::directory_iterator(app_dir)) {
            std::string board_filename = dir_entry.path();
            if (board_filename.ends_with(".xml")) {
                Board* cur_board = board_from_xml(board_filename);
                boards.push_back(cur_board);
                main_window.add_board(*cur_board);
            }
        }
    }

    add_window(main_window);
}

void ui::Application::on_activate() {
    Gtk::Application::on_activate();
    main_window.set_visible();
}