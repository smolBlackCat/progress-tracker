#include <app_info.h>
#include <iostream>

#include "ui/application.h"

/**
 * Progress app main entry.
*/
int main(int argc, char *argv[]) {
	std::cout << "Progress Tracker " << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;
	auto app = ui::Application::create();
	return app->run(argc, argv);
}