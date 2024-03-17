#include <app_info.h>
#include <libintl.h>

#include <cstdlib>
#include <iostream>
#include <locale>

#include "ui/application.h"

std::string get_locale_dir() {
    if (strcmp(BUILD_TYPE, "Release") == 0) {
        return strcmp(FLATPAK, "True") == 0? "/app/share/locale/":"/usr/share/locale/";
    } else {
        return std::string{getenv("PWD")} + "/locales/";
    }
}

/**
 * Progress app main entry.
 */
int main(int argc, char *argv[]) {
    std::setlocale(LC_ALL, "");
    std::string locale_dir = get_locale_dir();
    std::cout << locale_dir << std::endl;
    bindtextdomain("progress-tracker", locale_dir.c_str());
    textdomain("progress-tracker");

    std::cout << "Progress Tracker " << MAJOR_VERSION << "." << MINOR_VERSION
              << std::endl;
    auto app = ui::Application::create();
    return app->run(argc, argv);
}