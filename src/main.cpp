#include <app_info.h>
#include <iostream>

#include "ui/application.h"

/**
 * Progress app main entry.
*/
int main(int argc, char *argv[]) {
	std::cout << "Progress Tracker " << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;

	auto app = ui::Application::create();
	int return_code = app->run(argc, argv);
	std::cout << "Return Code: " << return_code << std::endl;

	return return_code;
}