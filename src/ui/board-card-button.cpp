#include "board-card-button.h"

#include <cstdint>
#include <filesystem>
#include <format>
#include <string>

uint32_t rgb_to_hex(const Gdk::RGBA& colour) {
    uint8_t r = static_cast<uint8_t>(colour.get_red_u());
    uint8_t g = static_cast<uint8_t>(colour.get_green_u());
    uint8_t b = static_cast<uint8_t>(colour.get_blue_u());
    uint8_t a = static_cast<uint8_t>(colour.get_alpha_u());

    return (r << 24) + (g << 16) + (b << 8) + a;
}

ui::BoardCardButton::BoardCardButton(std::string board_filepath)
    : Button{},
      root_box{Gtk::Orientation::VERTICAL},
      board_thumbnail{},
      board_name{},
      board_filepath{board_filepath} {
    if (!std::filesystem::exists(board_filepath))
        throw std::domain_error{"Fatal"};

    Board board{board_filepath};

    set_valign(Gtk::Align::CENTER);
    set_halign(Gtk::Align::CENTER);
    set_has_frame(false);

    set_expand(false);

    set_name_(board.get_name());
    board_name.set_valign(Gtk::Align::CENTER);
    board_name.set_vexpand(false);

    set_background(board.get_background());
    board_thumbnail.set_size_request(256, 256);
    board_thumbnail.set_margin_top(10);

    root_box.set_spacing(4);
    root_box.append(board_thumbnail);
    root_box.append(board_name);
    set_child(root_box);
}

std::string ui::BoardCardButton::get_filepath() { return board_filepath; }

void ui::BoardCardButton::update(Board* board) {
    board_filepath = board->get_filepath();
    set_name_(board->get_name());
    set_background(board->get_background());
}

void ui::BoardCardButton::set_name_(const std::string& name) {
    board_name.set_markup(std::string{std::format("<b>{}</b>", name)});
}

/**
 * TODO: The logic for setting background here is flawed. Add way out in case
 * the background (image file) gets deleted.
*/
void ui::BoardCardButton::set_background(const std::string& background) {
    if (std::filesystem::exists(background)) {
        auto board_bg_image = Gdk::Pixbuf::create_from_file(
            background, 256, 256, false);
        board_thumbnail.set(board_bg_image);
    } else {
        auto solid_colour =
            Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, 256, 256);
        Gdk::RGBA colour{background};
        solid_colour->fill(rgb_to_hex(colour));
        board_thumbnail.set(solid_colour);
    }
}