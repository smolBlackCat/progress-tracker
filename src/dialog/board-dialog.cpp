#include "board-dialog.h"

#include <adwaita.h>
#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <utils.h>

#include "core/colorable.h"

namespace ui {
// TODO: Add property signals for this base class for better extensibility
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

    g_signal_connect(
        board_dialog->gobj(), "closed",
        G_CALLBACK(+[](AdwDialog*, gpointer user_data) {
            spdlog::get("ui")->info("[BoardDialog.close] Board dialog closed");
        }),
        nullptr);
}

BoardDialog::~BoardDialog() {}

void BoardDialog::open(Gtk::Window& parent) {
    adw_dialog_present(ADW_DIALOG(board_dialog->gobj()),
                       static_cast<Gtk::Widget&>(parent).gobj());
    this->parent = &parent;

    spdlog::get("app")->info("A Board dialog opened");
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
        set_picture(rgba);
        spdlog::get("app")->info("[BoardDialog.set_picture] Color set to: {}",
                                 rgba.to_string().c_str());
    } catch (Gtk::DialogError& err) {
        spdlog::get("app")->warn("[BoardDialog.on_color_finish] {}", err.what());
    }
}

void BoardDialog::on_filedialog_finish(
    const Glib::RefPtr<Gio::AsyncResult>& result,
    const Glib::RefPtr<Gtk::FileDialog>& dialog) {
    try {
        image_filename = dialog->open_finish(result)->get_path();
        set_picture(compressed_thumb_filename(image_filename));
        spdlog::get("app")->info("[BoardDialog.set_picture] Picture set to: {}",
                                 image_filename);
    } catch (Glib::Error& err) {
        spdlog::get("app")->warn("[BoardDialog.on_filedialog_finish] {}",
                                err.what());
    }
}

// FIXME: Colour setting code is pretty ineficient because of the to-hex
// conversion overhead
void BoardDialog::set_picture(const Gdk::RGBA& rgba) {
    this->rgba = rgba;
    bg_type = BackgroundType::COLOR;
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
}

void BoardDialog::set_picture(const std::string& image_filename) {
    board_picture->set_filename(image_filename);
    bg_type = BackgroundType::IMAGE;
}
}  // namespace ui