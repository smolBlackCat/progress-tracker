#include <string>

/**
 * @brief Background compression image quality
 */
enum class ImageQuality { HIGH, MEDIUM, LOW };

/**
 * @brief Returns a compressed background image version from the image in
 * filename
 */
std::string compressed_bg_filename(const std::string& filename,
                                   ImageQuality quality = ImageQuality::MEDIUM);

/**
 * @brief Returns a thumbnail version of the image in filename
 */
std::string compressed_thumb_filename(const std::string& filename);
