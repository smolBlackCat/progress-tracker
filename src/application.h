#pragma once

#include <glibmm.h>
#include <gtkmm/application.h>

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

protected:
    Application();

    void on_startup() override;
    void on_activate() override;

    ProgressWindow* main_window = nullptr;
};
}  // namespace ui

