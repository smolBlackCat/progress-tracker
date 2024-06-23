#include "editable-label-header.h"

#include <glibmm/i18n.h>

#include "cardlist-widget.h"

namespace ui {
EditableLabelHeader::EditableLabelHeader() : EditableLabelHeader{""} {}

EditableLabelHeader::EditableLabelHeader(const std::string& label,
                                         const std::string& label_css_class,
                                         const std::string& entry_css_class)
    : Gtk::Box{Gtk::Orientation::VERTICAL},
      menu{Gio::Menu::create()},
      actions{Gio::SimpleActionGroup::create()},
      key_controller{Gtk::EventControllerKey::create()},
      click_controller{Gtk::GestureClick::create()} {
    editing_box.set_spacing(4);

    if (entry_css_class != "") {
        entry.add_css_class(entry_css_class);
    }
    entry.set_valign(Gtk::Align::CENTER);
    entry.set_halign(Gtk::Align::START);
    entry.set_hexpand();
    entry.set_size_request(CardlistWidget::CARDLIST_SIZE -
                           confirm_changes_button.get_width());
    editing_box.append(entry);

    confirm_changes_button.set_valign(Gtk::Align::CENTER);
    confirm_changes_button.set_halign(Gtk::Align::END);
    confirm_changes_button.add_css_class("confirm-action");
    confirm_changes_button.set_hexpand();

    cancel_changes_button.set_valign(Gtk::Align::CENTER);
    cancel_changes_button.set_halign(Gtk::Align::END);
    cancel_changes_button.add_css_class("destructive-action");
    cancel_changes_button.set_hexpand();

    editing_box.append(confirm_changes_button);
    editing_box.append(cancel_changes_button);

    revealer.set_child(editing_box);
    revealer.set_transition_type(Gtk::RevealerTransitionType::SWING_DOWN);
    revealer.set_halign(Gtk::Align::CENTER);
    revealer.set_hexpand();
    append(revealer);

    label_box.set_spacing(4);
    label_box.append(this->label);
    label_box.append(menu_button);
    append(label_box);

    if (label_css_class != "") {
        this->label.add_css_class(label_css_class);
    }
    this->label.set_label(label);
    this->label.set_xalign(0);
    this->label.set_hexpand(false);
    this->label.set_halign(Gtk::Align::START);
    this->label.set_wrap();
    this->label.set_wrap_mode(Pango::WrapMode::WORD_CHAR);
    this->label.set_size_request(CardlistWidget::CARDLIST_SIZE -
                                 menu_button.get_width());
    entry.set_text(label);

    confirm_changes_button.set_icon_name("object-select-symbolic");
    confirm_changes_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::EditableLabelHeader::on_confirm_changes));

    cancel_changes_button.set_icon_name("process-stop-symbolic");
    cancel_changes_button.signal_clicked().connect(
        sigc::mem_fun(*this, &EditableLabelHeader::on_cancel_changes));

    add_option("edit", _("Rename"),
               sigc::mem_fun(*this, &ui::EditableLabelHeader::to_editing_mode));
    menu_button.insert_action_group("label-header", actions);
    menu_button.set_menu_model(menu);
    menu_button.set_icon_name("view-more-horizontal-symbolic");
    menu_button.set_valign(Gtk::Align::START);
    menu_button.set_hexpand();
    menu_button.set_halign(Gtk::Align::END);

    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (revealer.get_child_revealed()) {
                switch (keyval) {
                    case (GDK_KEY_Return): {
                        on_confirm_changes();
                        break;
                    }
                    case (GDK_KEY_Escape): {
                        on_cancel_changes();
                        break;
                    }
                }
            }
        },
        false);
    click_controller->signal_released().connect(
        [this](int n_press, double x, double y) {
            if (n_press >= 2 && (!revealer.get_child_revealed())) {
                to_editing_mode();
            }
        },
        false);
    add_controller(key_controller);
    add_controller(click_controller);
}

std::string EditableLabelHeader::get_text() { return label.get_text(); }

void EditableLabelHeader::set_label(const std::string& new_label) {
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
    label.set_visible();
    menu_button.set_visible();
}

sigc::signal_with_accumulator<void, void, std::string>&
EditableLabelHeader::signal_confirm() {
    return on_confirm_signal;
}

sigc::signal_with_accumulator<void, void, std::string>&
EditableLabelHeader::signal_cancel() {
    return on_cancel_signal;
}

void EditableLabelHeader::on_confirm_changes() {
    set_label(entry.get_text());
    exit_editing_mode();
    on_confirm_signal(label.get_text());
}

void EditableLabelHeader::on_cancel_changes() {
    exit_editing_mode();
    set_label(label.get_text());
    on_cancel_signal(label.get_text());
}

void EditableLabelHeader::add_option(
    const std::string& name, const std::string& title_name,
    const Gio::ActionMap::ActivateSlot& procedure) {
    actions->add_action(name, procedure);
    menu->append(title_name, std::string{"label-header"} + "." + name);
}
}  // namespace ui
