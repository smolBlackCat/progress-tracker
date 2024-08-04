#pragma once

#include <gtkmm.h>

#include <string>

namespace ui {

/**
 * @brief A custom label widget that's editable and extensible
 *
 * @details It works as a better EditableLabel from GTK. It supports both
 * editing and renaming, and new actions can be added to the options button
 * using add_option method.
 *
 * Behaviour:
 *
 *      * Double click: Enters renaming mode.
 *      * Enter click: Exits renaming mode if the header has
 *                     entered into renaming mode.
 */
class EditableLabelHeader : public Gtk::Box {
public:
    EditableLabelHeader();
    EditableLabelHeader(const std::string& label,
                        const std::string& label_css_class = "",
                        const std::string& entry_css_class = "");

    std::string get_text();

    virtual void set_label(const std::string& new_label);

    /**
     * @brief Add an option to the EditableLabelHeader main menu
     */
    void add_option_button(const std::string& title_name,
                           const std::string& name,
                           const Glib::SlotSpawnChildSetup& procedure);

    /**
     * @brief Changes the widget view to editing mode
     */
    void to_editing_mode();

    /**
     * @brief Changes the widget view out of editing mode
     */
    void exit_editing_mode();

    sigc::signal_with_accumulator<void, void, std::string>& signal_confirm();

    sigc::signal_with_accumulator<void, void, std::string>& signal_cancel();

protected:
    Gtk::Box editing_box{Gtk::Orientation::HORIZONTAL},
        label_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Revealer revealer;

    Gtk::Label label;
    Gtk::Entry entry;
    Gtk::Button confirm_changes_button, cancel_changes_button;

    Glib::RefPtr<Gio::SimpleActionGroup> menu_actions;
    Glib::RefPtr<Gio::Menu> menu;
    Gtk::MenuButton menu_button;

    Glib::RefPtr<Gtk::EventControllerKey> key_controller;
    Glib::RefPtr<Gtk::GestureClick> click_controller;

    sigc::signal_with_accumulator<void, void, std::string> on_confirm_signal;
    sigc::signal_with_accumulator<void, void, std::string> on_cancel_signal;

    /**
     * @brief Updates the label
     */
    void on_confirm_changes();

    /**
     * @brief Cancels the changes made to this label
     */
    void on_cancel_changes();
};
}  // namespace ui

