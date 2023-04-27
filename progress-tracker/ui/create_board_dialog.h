#pragma once
#include <map>
#include <string>

#include "gdkmm/rgba.h"
#include "giomm/asyncresult.h"
#include "giomm/liststore.h"
#include "glibmm/refptr.h"
#include "gtkmm/alertdialog.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/colordialog.h"
#include "gtkmm/entry.h"
#include "gtkmm/error.h"
#include "gtkmm/filedialog.h"
#include "gtkmm/headerbar.h"
#include "gtkmm/label.h"
#include "gtkmm/window.h"
#include "gtkmm/colordialogbutton.h"

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
    CreateBoardDialog();
    std::map<std::string, std::string> get_entry() const noexcept;

private:
    /**
     * @brief Reorders selected_bg_label and select_bg_button below the the
     * solid colour option radiobutton.
     */
    void on_solid_radiobutton_toggle();

    /**
     * @brief Reorders selected_bg_label and select_bg_button below the the
     * image option radiobutton.
     */
    void on_image_radiobutton_toggle();

    /**
     * @brief Handles the button click event for the background selection
     * button.
     *
     * This method is a callback function that is triggered when the background
     * selection button is clicked in the UI. It determines whether the solid
     * color checkbox is active or not, and opens either a color dialog or a
     * file dialog accordingly to let the user choose the background for the
     * board.
     *
     * @remarks If the solid color checkbox is active, a color dialog is opened
     * to let the user choose a color. If the solid color checkbox is not
     *          active, a file dialog is opened to let the user choose an image
     * file (JPEG, PNG, or TIFF) as the background for the board.
     *
     * @see on_colourdialog_finish()
     * @see on_filedialog_finish()
     */
    void on_bg_button_click();

    /**
     * @brief Stores the entry into a map so the user is able to request it by
     * using the get_entry() method.
     */
    void save_entry();

    /**
     * @brief Closes the dialog window.
     */
    void close_window();

    // Helpers
    void setup_dialog_titlebar();
    void setup_dialog_root();

    // Dialog helpers
    /**
     * @brief Callback that stores the selected colour into selected_colour;
     */
    void on_colourdialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result);

    /**
     * @brief Callback that stores the selected file into selected_file;
     */
    void on_filedialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result,
                              const Glib::RefPtr<Gtk::FileDialog>& dialog);

    // Root Widgets
    Gtk::HeaderBar header_bar;
    Gtk::Box root, checkbuttons_box;
    Gtk::Label bg_label, selected_bg_label;
    Gtk::CheckButton solid_colour_cbttn, image_cbttn;
    Gtk::Button select_bg_button, create_button, cancel_button;
    Gtk::Entry board_name_entry;
    Glib::RefPtr<Gtk::AlertDialog> alert_dialog;

    // Colour Dialog attributes helpers
    Gtk::ColorDialogButton dialog_colour_button;
    Glib::RefPtr<Gtk::ColorDialog> colour_dialog;

    // File Dialog attributes helpers
    Glib::RefPtr<Gio::File> selected_file;

    std::map<std::string, std::string> dialog_data;
};
}  // namespace ui