#pragma once

#include <vector>
#include <glibmm.h>
#include <gtkmm/application.h>

#include "window.h"
#include "../core/board.h"

namespace ui {

/**
 * @brief Progress Application class
 *
 * @details class will preload all the boards in the .config/progress directory
 * or create one if there isn't.
 */
class Application : public Gtk::Application {
public:
    static Glib::RefPtr<ui::Application> create();
    ~Application() override;

protected:
    Application();
    
    /**
     * @brief Load boards into the application
    */
    void on_startup() override;
    void on_activate() override;

private:
    ProgressWindow main_window;
    std::vector<Board*> boards;
};
}  // namespace ui