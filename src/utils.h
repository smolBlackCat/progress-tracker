#include <gdkmm/rgba.h>

#include <cstdint>
#include <string>

/**
 * @brief Returns Progress locale folder
 */
std::string locale_folder();

/**
 * @brief Progress boards folder path
 */
std::string progress_boards_folder();

/**
 * @brief Generates an unique filename based on a given base and the
 * filename's availability
 */
std::string gen_unique_filename(const std::string& base);

uint32_t rgb_to_hex(const Gdk::RGBA& color);

