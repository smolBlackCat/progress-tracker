#include <iostream>
#include <gtkmm-4.0/gtkmm/application.h>
#include <gtkmm-4.0/gtkmm/window.h>
#include "app_info.h"

class MyWindow : public Gtk::Window {
public:
	MyWindow();
};

MyWindow::MyWindow() {
	set_title("Progress Tracker");
	set_default_size(200, 200);
}

int main(int argc, char* argv[]) {
	std::cout << "Progress Tracker " << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;
	auto app = Gtk::Application::create("com.moura.progress-tracker");
	return app->make_window_and_run<MyWindow>(argc, argv);
}