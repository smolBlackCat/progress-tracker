#include "application.h"

#include <adwaita.h>
#include <app_info.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <utils.h>

#include <cstdlib>
#include <cstring>

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
    spdlog::get("app")->debug("Application started");
    adw_init();

    auto window_builder = Gtk::Builder::create_from_resource(PROGRESS_WINDOW);
    main_window = Gtk::Builder::get_widget_derived<ui::ProgressWindow>(
        window_builder, "app-window", progress_settings, m_manager);
    if (!main_window) {
        spdlog::get("app")->critical("Failed to create main window");
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

    for (const auto& local_entry : m_manager.local_boards()) {
        main_window->add_local_board_entry(local_entry);
    }

    add_window(*main_window);
}

void ui::Application::on_activate() {
    Gtk::Application::on_activate();
    main_window->set_visible();
    spdlog::get("app")->info("Application initialised");
}
