#include "board-card-button.h"

#include <spdlog/spdlog.h>

#include <string>

ui::BoardCardButton::BoardCardButton(LocalBoard board_entry)
    : Button{},
      root_box{Gtk::Orientation::VERTICAL},
      board_thumbnail{},
      board_name{},
      local_board_entry{board_entry} {
    set_name_(board_entry.board->get_name());
    set_valign(Gtk::Align::CENTER);
    set_halign(Gtk::Align::CENTER);
    set_has_frame(false);

    set_expand(false);

    board_name.set_valign(Gtk::Align::CENTER);
    board_name.set_vexpand(false);

    set_background(board_entry.board->get_background());
    board_thumbnail.set_size_request(256, 256);
    board_thumbnail.set_margin_top(10);

    local_board_entry.board->signal_name_changed().connect(
        [this]() { set_name_(local_board_entry.board->get_name()); });
    local_board_entry.board->signal_background().connect(
        [this](std::string background) { set_background(background); });

    root_box.set_spacing(4);
    root_box.append(board_thumbnail);
    root_box.append(board_name);
    set_child(root_box);
}

time_point<system_clock, seconds> ui::BoardCardButton::get_last_modified()
    const {
    return local_board_entry.board->get_last_modified();
}

const std::shared_ptr<Board> ui::BoardCardButton::get_board() const {
    return local_board_entry.board;
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
            Color colour = string_to_color(background);
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
            Color colour{0, 0, 0, 1};
            solid_colour->fill(rgb_to_hex(colour));
            board_thumbnail.set(solid_colour);
            spdlog::get("ui")->warn(
                "[BoardCardButton] Cannot load background for Board \"{}\". "
                "Falling back to default color",
                board_name.get_text().c_str());
            break;
        }
    }
}

std::strong_ordering ui::BoardCardButton::operator<=>(
    const BoardCardButton& other) const {
    return this->get_last_modified() <=> other.get_last_modified();
}
