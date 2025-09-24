#pragma once

#include <glibmm.h>
#include <gtkmm/application.h>
#include <core/board-manager.h>

#include "window.h"

namespace ui {

/**
 * @brief Progress Application class
 */
class Application : public Gtk::Application {
public:
    static constexpr const char* PROGRESS_WINDOW =
        "/io/github/smolblackcat/Progress/app-window.ui";

    static Glib::RefPtr<ui::Application> create();

    ~Application() override;

protected:
    Application();

    void on_startup() override;
    void on_activate() override;

    BoardManager m_manager;
    ProgressWindow* main_window = nullptr;
    Glib::RefPtr<Gio::Settings> progress_settings;
};
}  // namespace ui
