#pragma once

#include <gtkmm.h>

#include <map>
#include <string>

namespace ui {

/**
 * @class CreateBoardDialog
 *
 * @brief Class representing a custom dialog window for getting the board's name
 * and the board's background.
 */
class CreateBoardDialog : public Gtk::Window {
public:
    /**
     * @brief Dialog Window constructor
     */
    CreateBoardDialog(BaseObjectType* cobject,
                      const Glib::RefPtr<Gtk::Builder>& ref_builder);

private:
    /**
     * @brief Stores the entry into a map so the user is able to request it by
     * using the get_entry() method.
     */
    void create_board();

    /**
     * @brief Closes the dialog window.
     */
    void close_window();

    void setup_dialog_buttons();

    void on_bg_button_click();

    /**
     * @brief Callback that stores the selected file into selected_file;
     */
    void on_filedialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result,
                              const Glib::RefPtr<Gtk::FileDialog>& dialog);

    /**
     * @brief Callback that stores the selected file into selected_colour;
     */
    void on_colourbutton_set();

    const Glib::RefPtr<Gtk::Builder>& builder;

    Gtk::Entry* p_board_name_entry;
    Gtk::Label* p_select_file_label;
    Gtk::Stack* p_background_selector_stack;
    Gtk::ColorDialogButton* p_colour_button;
    Gtk::Image* p_file_image;
    // File Dialog attributes helpers
    Glib::RefPtr<Gio::File> selected_file;
    Gdk::RGBA selected_colour;
};
}  // namespace ui