#pragma once

#include <gtkmm.h>

#include "../core/board.h"

namespace ui {

/**
 * @brief Button working as a frame to show the user the board's wallpaper
 */
class BoardCardButton : public Gtk::Button {
public:
    /**
     * @brief BoardCardButton constructor.
     * @param board Board object pointer from which basic information will be
     *              gathered from.
     */
    BoardCardButton(std::string board_filepath);

    std::string get_filepath();
    void update(Board* board);

private:
    Gtk::Box root_box;
    Gtk::Image board_thumbnail;
    Gtk::Label board_name;

    std::string board_filepath;
};
}  // namespace ui