#include <glibmm/i18n.h>

#include "cardlist-widget.h"
#include "editable-label-header.h"

namespace ui {
EditableLabelHeader::EditableLabelHeader() : EditableLabelHeader{""} {}

EditableLabelHeader::EditableLabelHeader(const std::string& label,
                                         const std::string& label_css_class,
                                         const std::string& entry_css_class)
    : Gtk::Box{Gtk::Orientation::VERTICAL},
      menu{Gio::Menu::create()},
      menu_actions{Gio::SimpleActionGroup::create()},
      key_controller{Gtk::EventControllerKey::create()},
      click_controller{Gtk::GestureClick::create()},
      focus_controller{Gtk::EventControllerFocus::create()} {
    editing_box.set_spacing(BOX_SPACING);

    if (entry_css_class != "") {
        entry.add_css_class(entry_css_class);
    }
    entry.set_valign(Gtk::Align::CENTER);
    entry.set_halign(Gtk::Align::START);
    entry.set_hexpand();
    entry.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH -
                           confirm_changes_button.get_width());
    focus_controller->signal_leave().connect(
        sigc::mem_fun(*this, &EditableLabelHeader::exit_editing_mode));
    entry.add_controller(focus_controller);
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

    label_box.set_spacing(BOX_SPACING);
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
    this->label.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH -
                                 menu_button.get_width());
    entry.set_text(label);

    confirm_changes_button.set_icon_name("object-select-symbolic");
    confirm_changes_button.signal_clicked().connect(
        sigc::mem_fun(*this, &ui::EditableLabelHeader::on_confirm_changes));

    cancel_changes_button.set_icon_name("process-stop-symbolic");
    cancel_changes_button.signal_clicked().connect(
        sigc::mem_fun(*this, &EditableLabelHeader::on_cancel_changes));

    add_option_button(_("Rename"), "rename", [this]() {
        // Interacting with the popover may cause the card to lose focus thus
        // we need to disable and the enable it again before the rename
        this->entry.remove_controller(focus_controller);
        this->to_editing_mode();
        this->entry.add_controller(focus_controller);
    });

    menu_button.insert_action_group("label-header", menu_actions);
    menu_button.set_menu_model(menu);
    menu_button.set_icon_name("view-more-horizontal-symbolic");
    menu_button.set_valign(Gtk::Align::START);
    menu_button.set_hexpand();
    menu_button.set_halign(Gtk::Align::END);
    menu_button.set_can_focus(false);
    menu_button.set_has_frame(false);

    key_controller->signal_key_released().connect(
        sigc::mem_fun(*this, &EditableLabelHeader::on_key_released), false);
    click_controller->signal_released().connect(
        sigc::mem_fun(*this, &EditableLabelHeader::on_mouse_button_released),
        false);
    add_controller(key_controller);
    add_controller(click_controller);
}

std::string EditableLabelHeader::get_text() const { return label.get_text(); }

void EditableLabelHeader::set_label(const std::string& new_label) {
    label.set_label(new_label);
    entry.set_text(new_label);
}

void EditableLabelHeader::add_option_button(
    const std::string& title_name, const std::string& name,
    const Glib::SlotSpawnChildSetup& procedure) {
    menu_actions->add_action(name, procedure);
    menu->append(title_name, std::string{"label-header"} + "." + name);
}

void EditableLabelHeader::to_editing_mode() {
    label.set_visible(false);
    menu_button.set_visible(false);
    revealer.set_reveal_child();
    entry.grab_focus();
}

void EditableLabelHeader::exit_editing_mode() {
    revealer.set_reveal_child(false);
    label.set_visible();
    menu_button.set_visible();
}

Glib::RefPtr<Gio::Menu> EditableLabelHeader::get_menu_model() const {
    return menu;
}

Glib::RefPtr<Gio::SimpleActionGroup> EditableLabelHeader::get_menu_actions()
    const {
    return menu_actions;
}

sigc::signal<void(std::string)>& EditableLabelHeader::signal_on_confirm() {
    return on_confirm_signal;
}

sigc::signal<void(std::string)>& EditableLabelHeader::signal_on_cancel() {
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

void EditableLabelHeader::on_key_released(guint keyval, guint keycode,
                                          Gdk::ModifierType state) {
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
}

void EditableLabelHeader::on_mouse_button_released(int n_press, double x,
                                                   double y) {
    if (n_press >= 1 && (!revealer.get_child_revealed())) {
        to_editing_mode();
    }
}
}  // namespace ui

