#include <app_info.h>
#include <libintl.h>

#include <filesystem>
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
#ifndef DEBUG
#ifdef FLATPAK
    return "/app/share/locale/";
#elif defined(WINDOWS) && defined(PORTABLE)
    return (std::filesystem::current_path() / "locale").string() +
           std::string{std::filesystem::path::preferred_separator};
#elif defined(WINDOWS)
    return std::string{std::getenv("PROGRAMFILES")} + "\\Progress\\locale\\";
#else
    return "/usr/share/locale/";
#endif
#else
    return (std::filesystem::current_path() / "locales").string() +
           std::string{std::filesystem::path::preferred_separator};
#endif
}

/**
 * Progress app main entry.
 */
int main(int argc, char *argv[]) {
    std::setlocale(LC_ALL, "");
    bindtextdomain("progress-tracker", get_locale_dir().c_str());
    bind_textdomain_codeset("progress-tracker", "utf-8");
    textdomain("progress-tracker");

    auto app = ui::Application::create();
    return app->run(argc, argv);
}