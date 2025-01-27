#include "board-dialog.h"

#include <adwaita.h>
#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>

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
    set_picture(Gdk::RGBA{0, 0, 0, 0});
    auto group = Gio::SimpleActionGroup::create();

    group->add_action("set-color",
                      sigc::mem_fun(*this, &BoardDialog::on_set_color));
    group->add_action("set-image",
                      sigc::mem_fun(*this, &BoardDialog::on_set_image));
    background_setter_menubutton->insert_action_group("board-dialog", group);
    footer_button->signal_clicked().connect(
        sigc::mem_fun(*this, &BoardDialog::on_footer_button_click));

    g_signal_connect(board_dialog->gobj(), "closed",
                     G_CALLBACK(+[](AdwDialog*, gpointer user_data) {
                         spdlog::get("ui")->info("Board Dialog closed");
                     }),
                     nullptr);
}

BoardDialog::~BoardDialog() {}

void BoardDialog::open(Gtk::Window& parent) {
    adw_dialog_present(ADW_DIALOG(board_dialog->gobj()),
                       static_cast<Gtk::Widget&>(parent).gobj());
    this->parent = &parent;

    spdlog::get("ui")->info("Board dialog opened");
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

    spdlog::get("ui")->info("Board dialog's file dialog opened");
}

void BoardDialog::on_set_color() {
    color_dialog->set_modal();
    color_dialog->choose_rgba(
        *parent, sigc::mem_fun(*this, &BoardDialog::on_color_finish));

    spdlog::get("ui")->info("Board dialog's color dialog opened");
}

void BoardDialog::on_color_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        rgba = color_dialog->choose_rgba_finish(result);
        bg_type = BackgroundType::COLOR;
        set_picture(rgba);
    } catch (Gtk::DialogError& err) {
        spdlog::get("ui")->warn(
            "Board Dialog has failed when finishing selecting a color: {}",
            err.what());
    }

    spdlog::get("ui")->debug("Board color set in the board dialog: {}",
                             rgba.to_string().c_str());
}

void BoardDialog::on_filedialog_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result,
    const Glib::RefPtr<Gtk::FileDialog>& dialog) {
    try {
        image_filename = dialog->open_finish(result)->get_path();
        bg_type = BackgroundType::IMAGE;
        set_picture(image_filename);
    } catch (Glib::Error& err) {
        spdlog::get("ui")->warn(
            "Board Dialog has failed when finishing selecting a file: {}",
            err.what());
    }

    spdlog::get("ui")->debug("Board image set in the board dialog: {}",
                             image_filename);
}

// FIXME: Colour setting code is pretty inneficient because of the to-hex
// conversion overhead
void BoardDialog::set_picture(const Gdk::RGBA& rgba) {
    this->rgba = rgba;
    auto color_frame_pixbuf =
        Gdk::Pixbuf::create(Gdk::Colorspace::RGB, false, 8, 30, 30);
    auto c = Color{rgba.get_red() * 255, rgba.get_green() * 255,
                   rgba.get_blue() * 255, rgba.get_alpha()};
    color_frame_pixbuf->fill(rgb_to_hex(c));
    if (board_picture->get_paintable()) {
        board_picture->set_paintable(nullptr);
    }
    board_picture->set_paintable(
        Gdk::Texture::create_for_pixbuf(color_frame_pixbuf));

    spdlog::get("ui")->info("Board picture set to color in the board dialog");
}

void BoardDialog::set_picture(const std::string& image_filename) {
    board_picture->set_filename(image_filename);

    spdlog::get("ui")->info("Board picture set to image in the board dialog");
}
}  // namespace ui