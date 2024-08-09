#include "utils.h"

#include <app_info.h>

#include <algorithm>
#include <filesystem>
#include <random>

std::string locale_folder() {
    return std::filesystem::path(LOCALE_FOLDER).string();
}

std::string progress_boards_folder() {
#ifdef FLATPAK
    return std::string{std::getenv("XDG_CONFIG_HOME")} + "/progress/boards/";
#elif WIN32
    return std::string{std::getenv("APPDATA")} + "\\Progress\\Boards\\";
#else
    return std::string{std::getenv("HOME")} + "/.config/progress/boards/";
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

uint32_t rgb_to_hex(const Gdk::RGBA& color) {
    uint8_t r = static_cast<uint8_t>(color.get_red_u());
    uint8_t g = static_cast<uint8_t>(color.get_green_u());
    uint8_t b = static_cast<uint8_t>(color.get_blue_u());
    uint8_t a = static_cast<uint8_t>(color.get_alpha_u());

    return (r << 24) + (g << 16) + (b << 8) + a;
}
