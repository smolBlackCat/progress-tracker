#include <libintl.h>

#include <filesystem>
#include <locale>

#include "application.h"
#include "utils.h"

/**
 * Progress app main entry.
 */
int main(int argc, char *argv[]) {
    std::setlocale(LC_ALL, "");
    bindtextdomain("progress-tracker", locale_folder().c_str());
    bind_textdomain_codeset("progress-tracker", "utf-8");
    textdomain("progress-tracker");

    Glib::setenv("GSK_RENDERER", "opengl");

    auto app = ui::Application::create();
    return app->run(argc, argv);
}