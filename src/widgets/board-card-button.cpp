#include "board-card-button.h"

#include <cstdint>
#include <filesystem>
#include <format>
#include <string>

#include "../utils.h"

ui::BoardCardButton::BoardCardButton(const std::string& board_filepath)
    : Button{},
      root_box{Gtk::Orientation::VERTICAL},
      board_thumbnail{},
      board_name{},
      board_filepath{board_filepath} {
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

std::string ui::BoardCardButton::get_filepath() const { return board_filepath; }

void ui::BoardCardButton::update(Board* board) {
    board_filepath = board->get_filepath();
    set_name_(board->get_name());
    set_background(board->get_background());
}

void ui::BoardCardButton::set_name_(const std::string& name) {
    board_name.set_markup(std::string{std::format("<b>{}</b>", name)});
}

void ui::BoardCardButton::set_background(const std::string& background) {
    BackgroundType bg_type = Board::get_background_type(background);

    switch (bg_type) {
        case BackgroundType::COLOR: {
            auto solid_colour =
                Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, 256, 256);
            Gdk::RGBA colour{background};
            solid_colour->fill(rgb_to_hex(colour));
            board_thumbnail.set(solid_colour);
            break;
        }
        case BackgroundType::IMAGE: {
            auto board_bg_image =
                Gdk::Pixbuf::create_from_file(background, 256, 256, false);
            board_thumbnail.set(board_bg_image);
            break;
        }
        case BackgroundType::INVALID: {
            auto solid_colour =
                Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, 256, 256);
            Gdk::RGBA colour{0, 0, 0, 1};
            solid_colour->fill(rgb_to_hex(colour));
            board_thumbnail.set(solid_colour);
            break;
        }
    }
}

std::strong_ordering ui::BoardCardButton::operator<=>(
    const BoardCardButton& other) const {
    auto this_lm_time = std::filesystem::last_write_time(board_filepath);
    auto other_lm_time = std::filesystem::last_write_time(other.get_filepath());

    return this_lm_time <=> other_lm_time;
}
