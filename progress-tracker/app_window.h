#include <gtkmm/window.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/menubutton.h>
#include <gtkmm/popover.h>

class ApplicationWindow : public Gtk::Window {

public:
    ApplicationWindow();
    ~ApplicationWindow() override;

    void show_about_dialog();

private:
    Gtk::HeaderBar header_bar;
    Gtk::MenuButton menu_button;    
};
