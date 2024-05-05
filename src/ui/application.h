#pragma once

#include <libadwaita-1/adwaita.h>
#include <glibmm.h>
#include <gtkmm/application.h>

#include <vector>

#include "../core/board.h"
#include "window.h"

namespace ui {

/**
 * @brief Progress Application class
 */
class Application : public Gtk::Application {
public:
    static Glib::RefPtr<ui::Application> create();

protected:
    Application();

    void on_startup() override;
    void on_activate() override;

    ProgressWindow* main_window = nullptr;
};
}  // namespace ui