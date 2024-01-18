#pragma once

#include <gtkmm.h>

#include <string>

namespace ui {

/**
 * @brief A custom label widget that's editable and extensible
 */
class EditableLabelHeader : public Gtk::Box {
public:
    EditableLabelHeader();
    EditableLabelHeader(std::string label);

    const std::string get_text();

    virtual void set_label(std::string new_label);

    /**
     * @brief Add an extra option to be selected in the menu button
     */
    void add_option(const std::string name, const std::string title_name,
                    const Gio::ActionMap::ActivateSlot& procedure);

protected:
    Gtk::Revealer revealer;
    Gtk::Label label;
    Gtk::Entry entry;
    Gtk::Button confirm_changes_button;
    Gtk::MenuButton menu_button;
    Glib::RefPtr<Gio::SimpleActionGroup> actions;
    Glib::RefPtr<Gio::Menu> menu;

    /**
     * @brief Changes the widget view to editing mode
     */
    void to_editing_mode();

    /**
     * @brief Changes the widget view out of editing mode
     */
    void exit_editing_mode();

    /**
     * @brief Updates the label
     */
    virtual void on_confirm_changes();
};
}  // namespace ui