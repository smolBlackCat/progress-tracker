#include <app_info.h>
#include <libintl.h>

#include <filesystem>
#include <iostream>
#include <locale>

#include "ui/application.h"

/**
 * @brief Return the app's locale directory.
 *
 * @details A different locale directory is returned depending on the build
 * settings.
 *
 * @return A string object containing
 */
std::string get_locale_dir() {
    if (strcmp(BUILD_TYPE, "Release") == 0) {
        if (strcmp(FLATPAK, "True") == 0) {
            return "/app/share/locale/";
        } else {
            return "/usr/share/locale/";
        }
    } else {
        return (std::filesystem::current_path() / "locales").string() +
               std::string{std::filesystem::path::preferred_separator};
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

    // Required for correctly decoding text on Windows
    bind_textdomain_codeset("progres-tracker", "utf-8");

    textdomain("progress-tracker");

    std::cout << "Progress Tracker " << MAJOR_VERSION << "." << MINOR_VERSION
              << std::endl;
    auto app = ui::Application::create();
    return app->run(argc, argv);
}