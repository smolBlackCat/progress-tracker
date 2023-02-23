#include <gtkmm/application.h>
#include <app_info.h>
#include <iostream>
#include "ui/window.h"

/**
 * Progress app main entry.
*/
int main(int argc, char *argv[])
{
	std::cout << "Progress Tracker " << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;

	auto app = Gtk::Application::create("com.moura.Progress");

	return app->make_window_and_run<ui::ProgressWindow>(argc, argv);
}