#pragma once

#include <core/board-manager.h>
#include <gtkmm.h>

namespace ui {
class CardlistWidget;

/**
 * @brief Widget that holds a list of CardLists
 */
class BoardWidget : public Gtk::ScrolledWindow {
public:
    static constexpr const char* CSS_FORMAT =
        "#board-root {transition-property: background-image, "
        "background-color;}";
    static constexpr const char* CSS_FORMAT_FILE =
        "#board-root {{transition-property: background-image, "
        "background-color; "
        "background-size: cover;"
        "background-repeat: no-repeat;"
        "background-image: url(\"file:{}\");}}";
    static constexpr const char* CSS_FORMAT_RGB =
        "#board-root {{transition-property: background-image, "
        "background-color; "
        "background-color: {};}}";

    static constexpr int SAVE_INTERVAL = 1000 * 10;
    static constexpr int UPDATE_INTERVAL = SAVE_INTERVAL * 6;
    static constexpr int SCROLL_SPEED_FACTOR = 6;

    Gtk::Box m_root;

    /**
     * @brief BoardWidget constructor
     *
     * @param manager BoardManager reference
     */
    BoardWidget(BoardManager& manager);

    /**
     * @brief Loads all widgets composing a BoardWidget from a board object
     *
     * @param board shared pointer to the board object
     */
    void set(const std::shared_ptr<Board>& board);

    /**
     * @brief Updates board' name
     *
     * @param board_name new name for the board
     */
    void set_name(const std::string& board_name);

    /**
     * @brief Sets board widget background
     *
     * @param background string referring to a background, either a colour
     * code or a filename
     */
    void set_background(const std::string& background);

    /**
     * @brief Reorders two CardlistWidget objects.
     *
     * @param next widget to be put after sibling
     * @param sibling widget to be put before next
     */
    void reorder_cardlist(CardlistWidget& next, CardlistWidget& sibling);

    /**
     * @brief Removes a CardlistWidget widget.
     *
     * @param cardlist reference to the cardlist to be removed.
     * @return true if the cardlist was successfully removed, false
     * otherwise
     */
    void remove_cardlist(ui::CardlistWidget& cardlist);

    /**
     * @brief Cleans the BoardWidget, thus deleting al widgets associated with
     * it
     */
    void clear();

    /**
     * @brief Calls BoardManager saving variants depending on the Board type
     * (e.g. local or external)
     *
     * @param clear indicates whether to clear the board widget after saving
     */
    void save(bool clear_after_save = true);

    /**
     * @brief Describes whether the board should be able to scroll
     * horizontally
     *
     * @param scroll boolean indicating whether horizontal scrolling should
     * be enabled
     */
    void set_scroll(bool scroll = true);

    /**
     * @brief Adds a new CardlistWidget widget based on the CardList object.
     *
     * @param cardlist CardList object
     * @param editing_mode bool indicating whether the cardlist is
     * completely new (has not been loaded from a file) or not
     * @return pointer to the newly added CardlistWidget
     */
    ui::CardlistWidget* add_cardlist(const CardList& cardlist,
                                     bool editing_mode = false);

    ui::CardlistWidget* insert_new_cardlist_after(const CardList& cardlist,
                                                  ui::CardlistWidget* sibling);

    /**
     * @brief Retrieves the background string
     *
     * @return reference to the background string
     */
    std::string get_background() const;

    /**
     * @brief Retrieves the board's name
     *
     * @return reference to the board's name string
     */
    std::string get_name() const;

    /**
     * @brief Returns true if the board is set up to horizontally scroll
     *
     * @return true if horizontal scrolling is enabled, false otherwise
     */
    bool scroll() const;

    /**
     * @brief Retrieves the current board object
     *
     * @return shared pointer to the current board object
     */
    std::shared_ptr<Board> board() const;

    CardlistWidget* __add_cardlist(const std::shared_ptr<CardList>& cardlist,
                                   bool editing_mode = false);

protected:
    void __setup_auto_scrolling();
    void __set_background(const std::string& background);

    BoardManager& m_manager;
    std::vector<sigc::connection> m_connections;

#ifdef WIN32
    Gtk::Overlay overlay;
    Gtk::Picture picture;
    Gtk::ScrolledWindow scr;
#endif
    Gtk::Button m_add_button;
    std::shared_ptr<Board> m_board = nullptr;
    Glib::RefPtr<Gtk::CssProvider> m_css_provider;
    std::vector<ui::CardlistWidget*> m_cardlists;
    double x, y;
    bool m_on_scroll = false;
};

}  // namespace ui

