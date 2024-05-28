#include "application.h"

#include <adwaita.h>
#include <app_info.h>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>

Glib::RefPtr<ui::Application> ui::Application::create() {
    return Glib::RefPtr<ui::Application>(new Application());
}

ui::Application::Application()
    : Gtk::Application{APPLICATION_ID} {}

void ui::Application::on_startup() {
    Gtk::Application::on_startup();
    adw_init();

    auto window_builder = Gtk::Builder::create_from_resource(PROGRESS_WINDOW);
    main_window = Gtk::Builder::get_widget_derived<ui::ProgressWindow>(
        window_builder, "app-window");
    if (!main_window) {
        exit(1);
    }

    #ifdef FLATPAK
        std::string app_dir =
            std::string{std::getenv("XDG_CONFIG_HOME")} + "/progress/boards";
    #elif WINDOWS
        std::string app_dir =
            std::string{std::getenv("APPDATA")} + "\\Progress\\Boards";
    #else
        std::string app_dir =
            std::string{std::getenv("HOME")} + "/.config/progress/boards";
    #endif

    if (!std::filesystem::exists(app_dir)) {
        if (!std::filesystem::create_directories(app_dir)) {
            std::cerr
                << "\033[;32mIt was not possible to create a directory\033[m"
                << std::endl;
        }
    } else {
        for (const auto& dir_entry :
             std::filesystem::directory_iterator(app_dir)) {
            std::string board_filename = dir_entry.path().string();
            if (board_filename.ends_with(".xml")) {
                try {
                    main_window->add_board(board_filename);
                } catch (std::invalid_argument& err) {
                    // TODO: Add code keeping track of how many failures to
                    // load boards
                    std::cerr << err.what() << std::endl;
                }
            }
        }
    }

    add_window(*main_window);
}

void ui::Application::on_activate() {
    Gtk::Application::on_activate();
    main_window->set_visible();
}
