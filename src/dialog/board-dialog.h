#pragma once

#include <core/board.h>
#include <gtkmm.h>

namespace ui {

/**
 * @brief Class implementing the board dialog.
 *
 * The BoardDialog class is responsible for displaying and managing the board
 * dialog within the UI. It provides functionalities such as setting the board
 * background image or color, and handling user interactions within the dialog.
 */
class BoardDialog {
public:
    /**
     * @brief Destructor.
     */
    virtual ~BoardDialog();

    /**
     * @brief Opens the board dialog.
     *
     * @param parent Reference to the parent Gtk::Window.
     */
    virtual void open(Gtk::Window& parent);

protected:
    /**
     * @brief Constructs a BoardDialog object.
     */
    BoardDialog();

    /**
     * @brief Handles the action to set the board background image.
     */
    void on_set_image();

    /**
     * @brief Handles the action to set the board background color.
     */
    void on_set_color();

    /**
     * @brief Callback function for when the file dialog operation finishes.
     *
     * @param result Reference to the asynchronous result.
     * @param dialog Reference to the file dialog.
     */
    void on_filedialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result,
                              const Glib::RefPtr<Gtk::FileDialog>& dialog);

    /**
     * @brief Callback function for when the color dialog operation finishes.
     *
     * @param result Reference to the asynchronous result.
     */
    void on_color_finish(const Glib::RefPtr<Gio::AsyncResult>& result);

    /**
     * @brief Handles the footer button click action.
     */
    virtual void on_footer_button_click() = 0;

    /**
     * @brief Sets the board background picture using an RGBA color.
     *
     * @param rgba RGBA color object.
     */
    void set_picture(const Gdk::RGBA& rgba);

    /**
     * @brief Sets the board background picture using an image file.
     *
     * @param image_filename Path to the image file.
     */
    void set_picture(const std::string& image_filename);

    Glib::RefPtr<Gtk::Builder>
        builder;  ///< Builder for constructing the UI elements.

    Gtk::Window* parent = nullptr;  ///< Pointer to the parent window.
    Glib::RefPtr<Glib::Object>
        board_dialog;  ///< Reference to the board dialog object.
    Glib::RefPtr<Gtk::ColorDialog>
        color_dialog;               ///< Reference to the color dialog.
    Gtk::Entry* board_title_entry;  ///< Entry widget for the board title.
    Gtk::MenuButton* background_setter_menubutton;  ///< Menu button for setting
                                                    ///< the background.
    Gtk::Picture*
        board_picture;  ///< Picture widget for displaying the board background.
    Gtk::Button* footer_button;  ///< Footer button for additional actions.

    Gdk::RGBA rgba;              ///< RGBA color for the board background.
    std::string image_filename;  ///< Filename of the background image.

    BackgroundType bg_type;  ///< Type of the background (color or image).

    static constexpr const char* BOARD_DIALOG =
        "/io/github/smolblackcat/Progress/board-dialog.ui";  ///< Path to the
                                                             ///< board dialog
                                                             ///< UI file.
};

}  // namespace ui