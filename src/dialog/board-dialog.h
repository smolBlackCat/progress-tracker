#pragma once

#include <core/board.h>
#include <gtkmm.h>

namespace ui {

class BoardDialog {
public:
    virtual ~BoardDialog();
    /**
     * @brief Opens the board dialog
     */
    void open(Gtk::Window& parent);

    /**
     * @brief Closes the window dialog. The dialog is then destroyed
     */
    void close();

protected:
    BoardDialog();
    void on_set_image();
    void on_set_color();

    void on_filedialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result,
                              const Glib::RefPtr<Gtk::FileDialog>& dialog);

    void on_color_finish(const Glib::RefPtr<Gio::AsyncResult>& result);

    virtual void on_footer_button_click() = 0;

    void set_picture(const Gdk::RGBA& rgba);
    void set_picture(const std::string& image_filename);

    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Window* parent = nullptr;
    Glib::RefPtr<Glib::Object> board_dialog;
    Glib::RefPtr<Gtk::ColorDialog> color_dialog;
    Gtk::Entry* board_title_entry;
    Gtk::MenuButton* background_setter_menubutton;
    Gtk::Picture* board_picture;
    Gtk::Button* footer_button;

    Gdk::RGBA rgba;
    std::string image_filename;

    BackgroundType bg_type;

    static constexpr const char* BOARD_DIALOG =
        "/io/github/smolblackcat/Progress/board-dialog.ui";
};
}  // namespace ui