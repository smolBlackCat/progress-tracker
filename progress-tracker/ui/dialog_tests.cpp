#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "create_board_dialog.h"
#include "gtkmm/application.h"
#include "gtkmm/button.h"
#include "gtkmm/window.h"

/**
 * TODO: Find a proper way of testing the UI.
 * Create an Application subclass would help.
 */

class Dummy : public Gtk::Window {
   public:
    Dummy() : open_dialog_btt{"Dialog"}, dialog{} {
        set_default_size(600, 400);
        dialog.set_transient_for(*this);
        dialog.set_modal();
        open_dialog_btt.signal_clicked().connect(
            sigc::mem_fun(*this, &Dummy::open_dialog)
        );
        set_child(open_dialog_btt);
    }

   private:
    void open_dialog(){
        dialog.set_visible();
    };

    std::map<std::string, std::string> get_entry() {
        return dialog.get_entry();
    }

    ui::CreateBoardDialog dialog;
    Gtk::Button open_dialog_btt;
};

TEST_CASE("Opening a simple create Board Dialog") {
    auto app = Gtk::Application::create("com.moura.dialogTest");
    int return_value =
        app->make_window_and_run<Dummy>(0, nullptr);

    std::map<std::string, std::string> expected = {{"title", ""},
                                                   {"background", ""}};

    CHECK(expected == expected);
}