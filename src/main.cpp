#include <app_info.h>
#include <application.h>
#include <libintl.h>
#include <utils.h>

#include <locale>

/**
 * Progress app main entry.
 */
int main(int argc, char *argv[]) {
    auto sys_locale = std::setlocale(LC_ALL, "");
    auto cur_loc = std::locale::global(std::locale{sys_locale});

    bindtextdomain("progress-tracker", locale_folder().c_str());
    bind_textdomain_codeset("progress-tracker", "utf-8");
    textdomain("progress-tracker");

#if WIN32
    Glib::setenv("GSK_RENDERER", "opengl");
#endif

    auto app = ui::Application::create();
    return app->run(argc, argv);
}
