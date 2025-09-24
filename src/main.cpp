#include <app_info.h>
#include <application.h>
#include <libintl.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <locale>

/**
 * Progress app main entry.
 */
int main(int argc, char* argv[]) {
#ifdef DEVELOPMENT
    spdlog::set_level(spdlog::level::debug);
#endif

    auto app_logger = spdlog::stdout_color_mt("app");
    auto ui_logger = spdlog::stdout_color_mt("ui");

    auto sys_locale = std::setlocale(LC_ALL, "");
    try {
        auto cur_loc = std::locale::global(std::locale{sys_locale});
    } catch (...) {
        auto cur_loc = std::locale::global(std::locale::classic());
    }

    bindtextdomain("progress-tracker", LOCALE_FOLDER);
    bind_textdomain_codeset("progress-tracker", "utf-8");
    textdomain("progress-tracker");

    spdlog::get("app")->info("Locale: {}", sys_locale);

    auto app = ui::Application::create();
    int code = app->run(argc, argv);

    spdlog::get("app")->info("Exit code: {}", code);
    return code;
}
