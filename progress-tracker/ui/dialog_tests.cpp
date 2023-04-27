#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "create_board_dialog.h"
#include "giomm/application.h"
#include "gtkmm/application.h"
#include "gtkmm/button.h"
#include "gtkmm/window.h"

class Dummy : public Gtk::Window {
   public:
    Dummy() :
    open_dialog_btt{"Insert progress-tracker as the board name\n"
    "Choose any option"}, dialog{} {
        set_default_size(600, 400);
        dialog.set_transient_for(*this);
        dialog.set_modal();
        open_dialog_btt.signal_clicked().connect(
            sigc::mem_fun(*this, &Dummy::open_dialog));
        set_child(open_dialog_btt);
    }

    std::map<std::string, std::string> get_entry() {
        return dialog.get_entry();
    }

   private:
    void open_dialog() { dialog.set_visible(); };

    ui::CreateBoardDialog dialog;
    Gtk::Button open_dialog_btt;
};

class DummyApplication : public Gtk::Application {
   public:
    static Glib::RefPtr<DummyApplication> create() {
        return Glib::RefPtr<DummyApplication>(new DummyApplication());
    }

    std::map<std::string, std::string> get_entry() {
        return dummy_window.get_entry();
    }

   protected:
    Dummy dummy_window{};

    void on_activate() override {
        add_window(dummy_window);
        dummy_window.set_visible();
    }

   protected:
    DummyApplication() : Gtk::Application("com.moura.test") {}
};

TEST_CASE("Opening a simple create Board Dialog") {
    auto app = DummyApplication::create();
    int return_value = app->run();

    CHECK(app->get_entry().at("title") == "progress-tracker");
    CHECK(app->get_entry().at("background").length() != 0);
}