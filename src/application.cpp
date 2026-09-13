#include "application.h"

#include <adwaita.h>
#include <app_info.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <utils.h>

#include <cstdlib>

#include "glibmm/priorities.h"

Glib::RefPtr<ui::Application> ui::Application::create() {
    return Glib::RefPtr<ui::Application>(new Application());
}

// BoardManager will load all boards in a different thread at the Application
// constructor.
ui::Application::Application()
    : Gtk::Application{APPLICATION_ID},
      m_settings{Gio::Settings::create("io.github.smolblackcat.Progress")} {}

ui::Application::~Application() { delete m_main_window; }

void ui::Application::on_startup() {
    Gtk::Application::on_startup();
    adw_init();

    auto window_builder = Gtk::Builder::create_from_resource(PROGRESS_WINDOW);
    m_main_window = Gtk::Builder::get_widget_derived<ui::ProgressWindow>(
        window_builder, "app-window", m_settings, m_manager);
    if (!m_main_window) {
        spdlog::get("app")->critical(
            "[Application.on_startup] App window could not be allocated");
        exit(1);
    }

    if (m_settings->get_boolean("window-maximized")) {
        m_main_window->maximize();
    } else {
        int window_height = m_settings->get_int("window-height");
        int window_width = m_settings->get_int("window-width");
        m_main_window->set_default_size(window_width, window_height);
    }

    add_window(*m_main_window);
}

void ui::Application::on_activate() {
    Gtk::Application::on_activate();
    // Currently, Progress only accepts one window per board session. This might
    // change in the future
    if (!m_on_session) {
        m_on_session = true;
        m_main_window->set_visible();
        // FIXME:
        // Scheduling an idle task that'll check whether all boards were loaded
        // before actually loading them into the application. This workaround
        // works but it can be enhanced by improving the core itself to be
        // thread-safe.
        Glib::signal_idle().connect(
            [this]() {
                if (m_manager.loaded()) {
                    for (const auto& local_entry : m_manager.local_boards()) {
                        m_main_window->add_local_board_entry(local_entry);
                    }
                    return false;
                } else {
                    return true;
                }
            },
            Glib::PRIORITY_LOW);
        spdlog::get("app")->info("App started");
    }
}
