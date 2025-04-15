#include <core/colorable.h>

#include <string>

/**
 * @brief Returns Progress locale folder
 */
std::string locale_folder();

/**
 * @brief Old Progress boards folder path. Used mainly for migration purposes.
 */
std::string progress_boards_folder_old();

/**
 * @brief Progress boards folder path
 */
std::string progress_boards_folder();

/**
 * @brief Generates an unique filename based on a given base and the
 * filename's availability
 */
std::string gen_unique_filename(const std::string& base);