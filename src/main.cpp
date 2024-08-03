#include <application.h>
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

    auto app = ui::Application::create();
    return app->run(argc, argv);
}
