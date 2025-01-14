#include "application.h"

#include <adwaita.h>
#include <app_info.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <utils.h>

#include <cstdlib>
#include <cstring>
#include <filesystem>

#include "core/board.h"

Glib::RefPtr<ui::Application> ui::Application::create() {
    return Glib::RefPtr<ui::Application>(new Application());
}

ui::Application::Application()
    : Gtk::Application{APPLICATION_ID},
      progress_settings{
          Gio::Settings::create("io.github.smolblackcat.Progress")} {}

ui::Application::~Application() { delete main_window; }

void ui::Application::on_startup() {
    Gtk::Application::on_startup();
    adw_init();

    auto window_builder = Gtk::Builder::create_from_resource(PROGRESS_WINDOW);
    main_window = Gtk::Builder::get_widget_derived<ui::ProgressWindow>(
        window_builder, "app-window", progress_settings);
    if (!main_window) {
        spdlog::get("app")->error("Failed to create main window");
        exit(1);
    }

    if (progress_settings->get_boolean("window-maximized")) {
        main_window->maximize();
        spdlog::get("app")->debug("Window maximized");
    } else {
        int window_height = progress_settings->get_int("window-height");
        int window_width = progress_settings->get_int("window-width");
        main_window->set_default_size(window_width, window_height);

        spdlog::get("app")->debug("Window size set to {}x{}", window_width,
                                  window_height);
    }

    std::string app_dir = progress_boards_folder();

    if (!std::filesystem::exists(app_dir)) {
        if (!std::filesystem::create_directories(app_dir)) {
            spdlog::get("app")->error(
                "Failed to create Progress Boards folder");
        }
    } else {
        for (const auto& dir_entry :
             std::filesystem::directory_iterator(app_dir)) {
            std::string board_filename = dir_entry.path().string();
            if (board_filename.ends_with(".xml")) {
                try {
                    BoardBackend backend{BackendType::LOCAL,
                                         std::map<std::string, std::string>{
                                             {"filepath", board_filename}}};
                    main_window->add_local_board(backend);
                } catch (std::invalid_argument& err) {
                    // TODO: Add code keeping track of how many failures to
                    // load boards
                    spdlog::get("app")->error("Failed to load board: {}",
                                              err.what());
                }
            }
        }
        spdlog::get("app")->info("Loaded all local boards from {}", app_dir);
    }

    add_window(*main_window);
}

void ui::Application::on_activate() {
    Gtk::Application::on_activate();
    main_window->set_visible();
    spdlog::get("app")->info("Application activated");
}
