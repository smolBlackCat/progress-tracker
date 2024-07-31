#pragma once
#include <gtkmm.h>

namespace ui {
class TaskWidget;
class CardWidget;

class CardDetailsDialog : public Gtk::Dialog {
public:
    CardDetailsDialog(BaseObjectType* cobject,
                      const Glib::RefPtr<Gtk::Builder>& builder);

    void remove_task(TaskWidget& task);
    void set_card_widget(CardWidget* card_widget);
    CardWidget* get_card_widget();

    static CardDetailsDialog* create();

protected:
    void on_add_button_click();
    void on_toggle();
    bool save();
    CardWidget* card_widget;

private:
    Gtk::Entry* task_name_entry;
    Gtk::Button* checklist_add_button;
    Gtk::ToggleButton* checklist_togglebutton;
    Gtk::Revealer* checklist_revealer;
    Gtk::Box* checklist_box;
    Glib::RefPtr<Gtk::TextBuffer> notes_textbuffer;
};
}  // namespace ui
