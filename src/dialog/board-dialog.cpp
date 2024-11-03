#include "board-dialog.h"

#include <adwaita.h>
#include <glibmm/i18n.h>
#include <sys/types.h>

#include "core/colorable.h"

namespace ui {

BoardDialog::BoardDialog()
    : builder{Gtk::Builder::create_from_resource(BOARD_DIALOG)},
      color_dialog{Gtk::ColorDialog::create()},
      board_dialog{builder->get_object("board-dialog")},
      board_title_entry{builder->get_widget<Gtk::Entry>("board-title-entry")},
      background_setter_menubutton{
          builder->get_widget<Gtk::MenuButton>("background-setter-menubutton")},
      board_picture{builder->get_widget<Gtk::Picture>("board-picture")},
      footer_button{builder->get_widget<Gtk::Button>("footer-button")} {
    g_signal_connect(board_dialog->gobj(), "close-attempt",
                     G_CALLBACK(+[](AdwDialog* self, gpointer data) {
                         reinterpret_cast<BoardDialog*>(data)->close();
                     }),
                     this);
    set_picture(Gdk::RGBA{0, 120, 212});
    auto group = Gio::SimpleActionGroup::create();

    group->add_action("set-color",
                      sigc::mem_fun(*this, &BoardDialog::on_set_color));
    group->add_action("set-image",
                      sigc::mem_fun(*this, &BoardDialog::on_set_image));
    background_setter_menubutton->insert_action_group("board-dialog", group);
    footer_button->signal_clicked().connect(
        sigc::mem_fun(*this, &BoardDialog::on_footer_button_click));
}

BoardDialog::~BoardDialog() {}

void BoardDialog::open(Gtk::Window& parent) {
    adw_dialog_present(ADW_DIALOG(board_dialog->gobj()),
                       static_cast<Gtk::Widget&>(parent).gobj());
    this->parent = &parent;
}

void BoardDialog::close() {
    adw_dialog_force_close(ADW_DIALOG(board_dialog->gobj()));
    // This line assumes that BoardDialog children will only be allocated to the
    // heap
    delete this;
}

void BoardDialog::on_set_image() {
    auto dialog = Gtk::FileDialog::create();

    dialog->set_title(_("Select a file"));
    dialog->set_modal();

    auto filters = Gio::ListStore<Gtk::FileFilter>::create();
    auto image_filter = Gtk::FileFilter::create();
    image_filter->set_name(_("Image Files"));
    image_filter->add_mime_type("image/png");
    image_filter->add_mime_type("image/jpeg");
    image_filter->add_mime_type("image/tiff");
    filters->append(image_filter);
    dialog->set_filters(filters);

    dialog->open(
        *parent,
        sigc::bind(sigc::mem_fun(*this, &ui::BoardDialog::on_filedialog_finish),
                   dialog));
}

void BoardDialog::on_set_color() {
    color_dialog->set_modal();
    color_dialog->choose_rgba(
        *parent, sigc::mem_fun(*this, &BoardDialog::on_color_finish));
}

void BoardDialog::on_color_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        rgba = color_dialog->choose_rgba_finish(result);
        bg_type = BackgroundType::COLOR;
        set_picture(rgba);
    } catch (Gtk::DialogError& err) {
        err.what();
    }
}

void BoardDialog::on_filedialog_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result,
    const Glib::RefPtr<Gtk::FileDialog>& dialog) {
    try {
        image_filename = dialog->open_finish(result)->get_path();
        bg_type = BackgroundType::IMAGE;
        set_picture(image_filename);
    } catch (Gtk::DialogError& err) {
        err.what();
    } catch (Glib::Error& err) {
        err.what();
    }
}

void BoardDialog::set_picture(const Gdk::RGBA& rgba) {
    auto color_frame_pixbuf =
        Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, 30, 30);
    color_frame_pixbuf->fill((static_cast<u_int8_t>(rgba.get_red_u()) << 24) |
                             (static_cast<u_int8_t>(rgba.get_green_u()) << 16) |
                             (static_cast<u_int8_t>(rgba.get_blue_u()) << 8) |
                             static_cast<u_int8_t>(1));
    if (board_picture->get_paintable()) {
        board_picture->set_paintable(nullptr);
    }
    board_picture->set_paintable(
        Gdk::Texture::create_for_pixbuf(color_frame_pixbuf));
}

void BoardDialog::set_picture(const std::string& image_filename) {
    board_picture->set_filename(image_filename);
}
}  // namespace ui