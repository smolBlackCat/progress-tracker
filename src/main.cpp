#include <application.h>
#include <app_info.h>
#include <libintl.h>
#include <utils.h>

/**
 * Progress app main entry.
 */
int main(int argc, char *argv[]) {
    std::setlocale(LC_ALL, "");
    bindtextdomain("progress-tracker", locale_folder().c_str());
    bind_textdomain_codeset("progress-tracker", "utf-8");
    textdomain("progress-tracker");

#if defined(WINDOWS)
    Glib::setenv("GSK_RENDERER", "opengl");
#endif

    auto app = ui::Application::create();
    return app->run(argc, argv);
}
