#include "board.h"

#include <chrono>
#include <filesystem>

#include "colorable.h"
#include "exceptions.h"
#include "guid.hpp"

namespace fs = std::filesystem;

const std::string Board::BACKGROUND_DEFAULT = "rgba(0,0,0,1)";

BackgroundType Board::get_background_type(const std::string& background) {
    if (fs::exists(background)) {
        return BackgroundType::IMAGE;
    }
    return BackgroundType::COLOR;
}

Board::Board(const std::string& name, const std::string& background,
             const xg::Guid uuid)
    : Item{name, uuid}, m_background{background} {
    if (!fs::exists(m_background)) {
        // Ensures background color is valid RGBA code
        m_background = color_to_string(string_to_color(background));
    }
}

Board::Board(const std::string& name, const std::string& background)
    : Board{name, background, xg::newGuid()} {}

Board::~Board() {}

void Board::set_name(const std::string& name) {
    if (!name.empty()) {
        Item::set_name(name);
        modify();
    }
}

void Board::set_background(const std::string& image_filename) {
    if (std::filesystem::exists(image_filename)) {
        m_background = image_filename;
        modify();
        m_background_signal.emit(m_background);
    }
}

void Board::set_background(const Color& color) {
    const std::string color_string = color_to_string(color);
    m_background = color_string;
    modify();
    m_background_signal.emit(m_background);
}

std::string Board::get_background() const { return m_background; }

time_point<system_clock, seconds> Board::get_last_modified() const {
    return m_last_modified;
}

bool Board::modified() const { return m_modified || m_cardlists.modified(); }

void Board::modify(bool m) { m_modified = m; }

ItemContainer<CardList>& Board::container() { return m_cardlists; }

sigc::signal<void(std::string)>& Board::signal_background() {
    return m_background_signal;
}
sigc::signal<void(std::string)>& Board::signal_description() {
    return m_description_signal;
}
