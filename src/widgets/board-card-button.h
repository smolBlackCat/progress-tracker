#pragma once

#include <core/board.h>
#include <gtkmm.h>

#include <chrono>
#include <compare>

namespace ui {

using namespace std::chrono;

/**
 * @brief Custom Gtk::Button implementation that allocates a Board object and
 *        opens the board view. It also shows the user to which Board the button
 *        will lead to when clicked by presenting the board's name and the
 *        board's background as a thumbnail.
 */
class BoardCardButton : public Gtk::Button {
public:
    /**
     * @brief BoardCardButton constructor.
     * @param board Board object pointer from which basic information will be
     *              gathered from.
     *
     * @throws std::invalid_argument when the file given does not exist
     */
    BoardCardButton(BoardBackend& boardbackend);

    /**
     * @brief Returns the filepath pointing to the Board object to be allocated
     *        when clicking this button.
     */
    time_point<system_clock, seconds> get_last_modified() const;

    void update(BoardBackend& board_backend);

    /**
     * @brief Updates the button's title
     */
    void set_name_(const std::string& name);

    /**
     * @brief Updates the button's thumbnail
     */
    void set_background(const std::string& background);

    BoardBackend get_backend() const;

    std::strong_ordering operator<=>(const BoardCardButton& other) const;

private:
    Gtk::Box root_box;
    Gtk::Image board_thumbnail;
    Gtk::Label board_name;
    time_point<system_clock, seconds> last_modified;
    BoardBackend board_backend;
};
}  // namespace ui
