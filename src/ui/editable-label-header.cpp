#include "editable-label-header.h"

#include "cardlist-widget.h"

#include <iostream>

namespace ui {
EditableLabelHeader::EditableLabelHeader() : EditableLabelHeader{""} {}

EditableLabelHeader::EditableLabelHeader(std::string label)
    : Gtk::Box{Gtk::Orientation::VERTICAL},
      revealer{},
      label{},
      entry{},
      confirm_changes_button{},
      menu_button{},
      menu{Gio::Menu::create()},
      actions{Gio::SimpleActionGroup::create()},
      key_controller{Gtk::EventControllerKey::create()},
      click_controller{Gtk::GestureClick::create()} {
    auto editing_box =
        Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    editing_box->set_spacing(4);
    entry.set_valign(Gtk::Align::CENTER);
    entry.set_halign(Gtk::Align::START);
    entry.set_hexpand();
    entry.set_size_request(CardlistWidget::CARDLIST_SIZE -
                           confirm_changes_button.get_width());
    editing_box->append(entry);
    confirm_changes_button.set_valign(Gtk::Align::CENTER);
    confirm_changes_button.set_halign(Gtk::Align::END);
    confirm_changes_button.set_hexpand();
    editing_box->append(confirm_changes_button);
    revealer.set_child(*editing_box);
    revealer.set_transition_type(Gtk::RevealerTransitionType::SWING_DOWN);
    revealer.set_halign(Gtk::Align::CENTER);
    revealer.set_hexpand();
    append(revealer);

    auto label_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    label_box->set_spacing(4);
    label_box->append(this->label);
    label_box->append(menu_button);
    append(*label_box);

    this->label.set_label(label);
    this->label.set_xalign(0);
    this->label.set_hexpand(false);
    this->label.set_halign(Gtk::Align::START);
    this->label.set_wrap();
    this->label.set_wrap_mode(Pango::WrapMode::WORD_CHAR);
    this->label.set_size_request(CardlistWidget::CARDLIST_SIZE -
                                 menu_button.get_width());
    entry.set_text(label);

    // Button setup code
    confirm_changes_button.set_icon_name("object-select-symbolic");
    confirm_changes_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::EditableLabelHeader::exit_editing_mode));

    add_option("edit", "Rename",
               sigc::mem_fun(*this, &ui::EditableLabelHeader::to_editing_mode));
    menu_button.insert_action_group("label-header", actions);
    menu_button.set_menu_model(menu);
    menu_button.set_icon_name("open-menu-symbolic");
    menu_button.set_valign(Gtk::Align::START);
    menu_button.set_hexpand();
    menu_button.set_halign(Gtk::Align::END);

    // Setting up Controllers
    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (keycode == 36 && revealer.get_child_revealed()) { // The user has clicked enter
                exit_editing_mode();
            }
        }, false
    );
    click_controller->signal_released().connect(
        [this](int n_press, double x, double y) {
            if (n_press >= 2 && (!revealer.get_child_revealed())) {
                to_editing_mode();
            }
        }
    );
    add_controller(key_controller);
    add_controller(click_controller);
}

const std::string EditableLabelHeader::get_text() { return label.get_text(); }

void EditableLabelHeader::set_label(std::string new_label) {
    label.set_label(new_label);
    entry.set_text(new_label);
};

void EditableLabelHeader::to_editing_mode() {
    label.set_visible(false);
    menu_button.set_visible(false);
    revealer.set_reveal_child();
}

void EditableLabelHeader::exit_editing_mode() {
    revealer.set_reveal_child(false);
    set_label(entry.get_text());
    label.set_visible();
    menu_button.set_visible();

    on_confirm_changes();
}

void EditableLabelHeader::on_confirm_changes() {}

void EditableLabelHeader::add_option(
    const std::string name, const std::string title_name,
    const Gio::ActionMap::ActivateSlot& procedure) {
    actions->add_action(name, procedure);
    menu->append(title_name, std::string{"label-header"} + "." + name);
}
}  // namespace ui