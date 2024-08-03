#pragma once

#include <core/board.h>
#include <gtkmm.h>

#include <compare>

namespace ui {

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
    BoardCardButton(const std::string& board_filepath);

    /**
     * @brief Returns the filepath pointing to the Board object to be allocated
     *        when clicking this button.
     */
    std::string get_filepath() const;

    /**
     * @brief Updates the Button settings
     *
     * @param board Board object pointer to load the settings from
     */
    void update(Board* board);

    /**
     * @brief Updates the button's title
     */
    void set_name_(const std::string& name);

    /**
     * @brief Updates the button's thumbnail
     */
    void set_background(const std::string& background);

    std::strong_ordering operator<=>(const BoardCardButton& other) const;

private:
    Gtk::Box root_box;
    Gtk::Image board_thumbnail;
    Gtk::Label board_name;

    std::string board_filepath;
};
}  // namespace ui
