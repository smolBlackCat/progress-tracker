#include <app_info.h>
#include <libintl.h>

#include <cstdlib>
#include <iostream>
#include <locale>

#include "ui/application.h"

/**
 * Progress app main entry.
 */
int main(int argc, char *argv[]) {
    std::setlocale(LC_ALL, "");
    std::string locale_dir = BUILD_TYPE == "Release"
                       ? "/usr/share/locale"
                       : (std::string{getenv("PWD")} + "/locales");
    bindtextdomain("progress-tracker", locale_dir.c_str());
    textdomain("progress-tracker");

    std::cout << "Progress Tracker " << MAJOR_VERSION << "." << MINOR_VERSION
              << std::endl;
    auto app = ui::Application::create();
    return app->run(argc, argv);
}