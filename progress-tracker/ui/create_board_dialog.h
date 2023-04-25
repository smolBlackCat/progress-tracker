#pragma once
#include <map>
#include <string>

#include "gdkmm/rgba.h"
#include "giomm/asyncresult.h"
#include "giomm/liststore.h"
#include "glibmm/refptr.h"
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

namespace ui {

class CreateBoardDialog : public Gtk::Window {
   public:
    CreateBoardDialog();
    std::map<std::string, std::string> get_entry() const noexcept;

   private:
    void on_solid_radiobutton_toggle();
    void on_image_radiobutton_toggle();
    void on_bg_button_click();
    void save_entry();
    void close_window();

    // Helpers
    void setup_dialog_titlebar();
    void setup_dialog_root();

    // Dialog helpers
    void on_colourdialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result);
    void on_filedialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result,
                              const Glib::RefPtr<Gtk::FileDialog>& dialog);

    // Root Widgets
    Gtk::HeaderBar header_bar;
    Gtk::Box root, checkbuttons_box;
    Gtk::Label bg_label, selected_bg_label;
    Gtk::CheckButton solid_colour_cbttn, image_cbttn;
    Gtk::Button select_bg_button, create_button, cancel_button;
    Gtk::Entry board_name_entry;

    // Colour Dialog attributes helpers
    Gdk::RGBA selected_colour;
    Glib::RefPtr<Gtk::ColorDialog> colour_dialog;

    // File Dialog attributes helpers
    Glib::RefPtr<Gio::File> selected_file;

    std::map<std::string, std::string> dialog_data;
};
}  // namespace ui