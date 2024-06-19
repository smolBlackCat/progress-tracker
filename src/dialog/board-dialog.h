#pragma once

#include <gtkmm.h>

namespace ui {

class BoardDialog : public Gtk::Window {
public:
    BoardDialog(BaseObjectType* cobject,
                const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~BoardDialog() = default;

    virtual void open_window();
    virtual void close_window();

protected:
    virtual void on_bg_button_click();
    virtual void on_filedialog_finish(
        const Glib::RefPtr<Gio::AsyncResult>& result,
        const Glib::RefPtr<Gtk::FileDialog>& dialog);
    virtual void on_colourbutton_set();

    // File Dialog attributes helpers
    Glib::RefPtr<Gio::File> selected_file;
    Gdk::RGBA selected_colour;
    bool file_selected = false;

    // Board Dialog Widgets
    Gtk::Entry* p_board_name_entry;
    Gtk::Label* p_select_file_label;
    Gtk::Stack* p_background_selector_stack;
    Gtk::ColorDialogButton* p_colour_button;
    Gtk::Image* p_file_image;
    Gtk::Button* p_left_button;
    Gtk::Button* p_right_button;
    Gtk::Button* p_select_file_button;
};
}  // namespace ui