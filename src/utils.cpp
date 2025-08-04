#include "utils.h"

#include <gdkmm/pixbuf.h>
#include <glibmm/checksum.h>
#include <spdlog/spdlog.h>

#include <filesystem>

namespace fs = std::filesystem;

std::string bg_cache_dir() {
#ifdef WIN32
    if (const char* local_app_data = std::getenv("LOCALAPPDATA")) {
        return std::string{local_app_data} + "\\Progress\\Backgrounds";
    }
#else
    if (const char* cache_home = std::getenv("XDG_CACHE_HOME")) {
        return std::string{cache_home} + "/progress/backgrounds";
    } else {
        return std::string{std::getenv("HOME")} +
               "/.cache/progress/backgrounds";
    }
#endif
}

std::string thumb_cache_dir() {
#ifdef WIN32
    if (const char* local_app_data = std::getenv("LOCALAPPDATA")) {
        return std::string{local_app_data} + "\\Progress\\Thumbnails";
    }
#else
    if (const char* cache_home = std::getenv("XDG_CACHE_HOME")) {
        return std::string{cache_home} + "/progress/thumbnails";
    } else {
        return std::string{std::getenv("HOME")} + "/.cache/progress/thumbnails";
    }
#endif
}

std::string compressed_bg_filename(const std::string& filename,
                                   ImageQuality quality) {
    std::string filename_checksum =
        Glib::Checksum::compute_checksum(Glib::Checksum::Type::MD5, filename);

    // Cache hit
    const fs::path cached_image = fs::path{bg_cache_dir()} / filename_checksum;
    if (fs::exists(cached_image)) {
        spdlog::get("ui")->debug("[BackgroundProvider] Cache hit");
    } else {
        // Compresses the given image file and return the compressed's filename
        spdlog::get("ui")->debug(
            "[Background] There was no compressed file available. "
            "Compress it");
        // We ensure a cache directory exists at all times
        fs::create_directories(bg_cache_dir());

        auto image_pixbuf = Gdk::Pixbuf::create_from_file(filename);
        Glib::RefPtr<Gdk::Pixbuf> compressed_image;

        switch (quality) {
            case ImageQuality::LOW: {
                compressed_image = image_pixbuf->scale_simple(
                    720, 480, Gdk::InterpType::BILINEAR);
                break;
            }
            case ImageQuality::MEDIUM: {
                compressed_image = image_pixbuf->scale_simple(
                    1280, 720, Gdk::InterpType::BILINEAR);
                break;
            }
            case ImageQuality::HIGH: {
                compressed_image = image_pixbuf->scale_simple(
                    1920, 1080, Gdk::InterpType::BILINEAR);
                break;
            }
        }

        compressed_image->save(cached_image.string(), "png");
    }

    return cached_image.string();
}

std::string compressed_thumb_filename(const std::string& filename) {
    std::string filename_checksum =
        Glib::Checksum::compute_checksum(Glib::Checksum::Type::MD5, filename);

    fs::create_directories(thumb_cache_dir());

    // Cache hit
    const fs::path cached_image =
        fs::path{thumb_cache_dir()} / filename_checksum;
    if (fs::exists(cached_image)) {
        spdlog::get("ui")->debug("[BackgroundProvider] Cache hit");
    } else {
        auto bg_pixbuf = Gdk::Pixbuf::create_from_file(filename);
        bg_pixbuf->scale_simple(256, 256, Gdk::InterpType::BILINEAR)
            ->save(cached_image.string(), "png");
    }
    return cached_image.string();
}
