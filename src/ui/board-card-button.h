#pragma once

#include <gtkmm.h>

#include "../core/board.h"

namespace ui {

/**
 * @brief Class for representing a widget for selecting a
 * kanban board.
*/
class BoardCardButton : public Gtk::Button {
public:
    /**
     * @brief BoardCardButton constructor
     * @param board Board object reference from which basic information will be
     *              gathered from
    */
    BoardCardButton(Board& board);

private:
    Gtk::Box root_box;
    Gtk::Image board_thumbnail;
    Gtk::Label board_name;
};
}