#include <app_info.h>
#include <application.h>
#include <libintl.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>
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

    const std::string lc_folder = std::filesystem::path(LOCALE_FOLDER).string();

    bindtextdomain("progress-tracker", lc_folder.c_str());
    bind_textdomain_codeset("progress-tracker", "utf-8");
    textdomain("progress-tracker");

    spdlog::get("app")->debug("Current System Locale: {}", sys_locale);
    spdlog::get("app")->debug("Loading locale data from: {}", lc_folder);

    auto app = ui::Application::create();
    int code = app->run(argc, argv);

    spdlog::get("app")->info("Application exited with code {}", code);
    return code;
}
