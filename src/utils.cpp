#include "utils.h"

#include <app_info.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <random>
#include <regex>

#ifdef DEVELOPMENT
constexpr const char* BOARDS_FOLDER = "/progress-debug/boards/";
constexpr const char* BOARDS_FOLDER_WIN32 = "\\Progress Debug\\Boards\\";
#else
constexpr const char* BOARDS_FOLDER = "/progress/boards/";
constexpr const char* BOARDS_FOLDER_WIN32 = "\\Progress\\Boards\\";
#endif

std::string locale_folder() {
    return std::filesystem::path(LOCALE_FOLDER).string();
}

std::string progress_boards_folder() {
#ifdef FLATPAK
    return std::string{std::getenv("XDG_CONFIG_HOME")} + BOARDS_FOLDER;
#elif WIN32
    return std::string{std::getenv("APPDATA")} + BOARDS_FOLDER_WIN32;
#else
    return std::string{std::getenv("HOME")} +
           std::format("/.config{}", BOARDS_FOLDER);
#endif
}

std::string format_basename(std::string basename) {
    std::transform(
        basename.begin(), basename.end(), basename.begin(),
        [](unsigned char c) { return c == ' ' ? '-' : std::tolower(c); });
    return basename;
}

// Not reliable(???)
std::string gen_unique_filename(const std::string& base) {
    std::string boards_dir = progress_boards_folder();
    std::string filename = "";
    if (std::filesystem::exists(boards_dir)) {
        std::string basename = format_basename(base);
        filename = boards_dir + basename + ".xml";
        if (std::filesystem::exists(filename)) {
            std::random_device r;
            std::default_random_engine e1(r());
            std::uniform_int_distribution<int> uniform_dist(1, 1000);
            int unique_id = uniform_dist(e1);
            filename =
                boards_dir + basename + std::to_string(unique_id) + ".xml";
            while (std::filesystem::exists(filename)) {
                unique_id = uniform_dist(e1);
                filename =
                    boards_dir + basename + std::to_string(unique_id) + ".xml";
            }
        }
    }
    return filename;
}