#pragma once

#include <core/board-manager.h>
#include <gtkmm.h>

#include <chrono>
#include <compare>

namespace ui {

using namespace std::chrono;

/**
 * @brief Custom Gtk::Button implementation that presents to the user the board
 * to be loaded. It also shows the user to which Board the button will lead to
 * when clicked by presenting the board's name and the board's background as a
 * thumbnail.
 */
class BoardCardButton : public Gtk::Button {
public:
    /**
     * @brief BoardCardButton constructor.
     * @param boardbackend Reference to a backend responsible for loading the
     * board object
     *
     * @throws std::invalid_argument if it is not possible to load information
     * about a board
     */
    BoardCardButton(LocalBoard board_entry);

    /**
     * @brief Updates the button's title
     */
    void set_name_(const std::string& name);

    /**
     * @brief Updates the button's thumbnail
     */
    void set_background(const std::string& background);

    const std::shared_ptr<Board> get_board() const;

    /**
     * @brief Gets the last modification time of the Board
     */
    time_point<system_clock, seconds> get_last_modified() const;

    std::strong_ordering operator<=>(const BoardCardButton& other) const;

private:
    Gtk::Box root_box;
    Gtk::Image board_thumbnail;
    Gtk::Label board_name;
    LocalBoard local_board_entry;
};
}  // namespace ui
