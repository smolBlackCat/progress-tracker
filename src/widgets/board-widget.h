#pragma once

#include <core/board-manager.h>
#include <gtkmm.h>

#include <utility>

namespace ui {
class CardlistWidget;

using CardlistWidgetLocation = std::pair<CardlistWidget*, int>;
/**
 * @brief Widget that holds a list of CardLists
 */
class BoardWidget : public Gtk::ScrolledWindow {
public:
    static constexpr const char* BOARD_BACKGROUND =
        "#board-root {transition-property: background-image, "
        "background-color;}";
    static constexpr const char* BOARD_BACKGROUND_IMAGE =
        "#board-root {{transition-property: background-image, "
        "background-color; "
        "background-size: cover;"
        "background-repeat: no-repeat;"
        "background-image: url(\"file:{}\");}}";
    static constexpr const char* BOARD_BACKGROUND_RGB =
        "#board-root {{transition-property: background-image, "
        "background-color; "
        "background-color: {};}}";

    static constexpr int SCROLL_SPEED_FACTOR = 6;

    /**
     * @brief BoardWidget constructor
     */
    BoardWidget();

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
     * @brief Adds a new cardlist widget
     *
     * @param child CardlistWidget object reference
     */
    void append(CardlistWidget& child);

    /**
     * @brief Inserts widget after sibling
     */
    void insert_after(CardlistWidget& widget, CardlistWidget& sibling);

    /**
     * @brief Reorders two CardlistWidget objects.
     *
     * @param next widget to be put after sibling
     * @param sibling widget to be put before next
     */
    void reorder(CardlistWidget& next, CardlistWidget& sibling);

    /**
     * @brief Removes a CardlistWidget widget.
     *
     * @param cardlist reference to the cardlist to be removed.
     * @return true if the cardlist was successfully removed, false
     * otherwise
     */
    void remove(ui::CardlistWidget& cardlist);

    /**
     * @brief Describes whether the board should be able to scroll
     * horizontally
     *
     * @param scroll boolean indicating whether horizontal scrolling should
     * be enabled
     */
    void set_scroll(bool scroll = true);

    /**
     * @brief Pops the cardlist out of this BoardWidget instance
     */
    void pop();

    /**
     * @brief Retrieves the background string
     *
     * @return board background, either a color code or a image path
     */
    const std::string& get_background() const;

    /**
     * @brief Retrieves the board's name
     *
     * @return board name
     */
    const std::string& get_name() const;

    /**
     * @brief Returns true if the board is set up to horizontally scroll
     *
     * @return true if horizontal scrolling is enabled, false otherwise
     */
    bool scroll() const;

    /**
     * @brief Returns true when BoardWidget is empty
     */
    bool empty() const;

    sigc::signal<void(std::string, std::string)>& signal_name_changed();
    sigc::signal<void(std::string, std::string)>& signal_background_changed();

    sigc::signal<void(CardlistWidget*, int)>& signal_added_cardlist();
    sigc::signal<void(CardlistWidget*)>& signal_remove_cardlist();
    sigc::signal<void(CardlistWidget*, CardlistWidget*, bool)>& signal_reorder();

protected:
    void __setup_auto_scrolling();

    std::string m_name;
    std::string m_background;

    sigc::connection m_timeout_scroller;

    sigc::signal<void(std::string, std::string)> m_name_changed_signal;
    sigc::signal<void(std::string, std::string)> m_background_changed_signal;

    sigc::signal<void(CardlistWidget*, int)> m_cardlist_added_signal;
    sigc::signal<void(CardlistWidget*)> m_cardlist_remove_signal;

    /**
     * void(next, sibling, up)
     *
     * `up` is a bit flag that indicates whether the next is put before or after
     * the sibling. This bit is true if and only if next is placed before
     * sibling, otherwise false.*/
    sigc::signal<void(CardlistWidget*, CardlistWidget*, bool)>
        m_cardlist_reorder_signal;

    sigc::signal<void()> m_scroll_changed_signal;

#ifdef WIN32
    Gtk::Overlay m_overlay;
    Gtk::Picture m_picture;
    Gtk::ScrolledWindow m_scr;
#endif

    Gtk::Box m_root;
    Gtk::Button m_add_button;
    Glib::RefPtr<Gtk::CssProvider> m_css_provider;

    // Necessary for drag-and-drop
    double m_x, m_y;
    bool m_on_scroll = false;
};

}  // namespace ui
