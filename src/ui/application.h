#pragma once

#include <glibmm.h>
#include <gtkmm/application.h>

#include <vector>

#include "../core/board.h"
#include "window.h"

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

    void add_board(Board* board);

protected:
    Application();

    void on_startup() override;
    void on_activate() override;

private:
    ProgressWindow main_window;
    std::vector<Board*> boards;
};
}  // namespace ui