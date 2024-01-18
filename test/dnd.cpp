#include <gtkmm.h>

#include <iostream>
#include <vector>

class AppWindow : public Gtk::Window {
public:
    AppWindow()
        : Gtk::Window{},
          box1{Gtk::Orientation::VERTICAL},
          box2{Gtk::Orientation::VERTICAL},
          add_item_button{"add new item"} {
        set_title("Drag and Drop Practise");
        set_default_size(800, 800);
        auto root_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        root_box->set_expand();
        root_box->append(add_item_button);
        add_item_button.signal_clicked().connect(sigc::mem_fun(*this, &AppWindow::on_add_item));
        set_child(*root_box);

        auto target_drop_controller_box1 = Gtk::DropTarget::create(Glib::Value<Gtk::Button*>::value_type(), Gdk::DragAction::MOVE);
        target_drop_controller_box1->signal_drop().connect(
            [this](const Glib::ValueBase& value, double x, double y) {
                std::cout << "Fatal, it's not working for some reason" << std::endl;
                if (G_VALUE_HOLDS(value.gobj(), Glib::Value<Gtk::Button*>::value_type())) {
                    Glib::Value<Gtk::Button*> button;
                    button.init(value.gobj());
                    auto transferred_button = button.get();
                    // TODO: I'll have to check if this button isn't in the listbox already
                    box2.remove(*transferred_button);
                    box1.append(*transferred_button);
                    std::cout << "Button added successfully" << std::endl;
                } else {
                    std::cout << "Wtf???" << std::endl;
                }
                return true;
            }
            ,false);
        box1.add_controller(target_drop_controller_box1);

        box1.set_expand();
        root_box->append(box1);

        auto target_drop_controller_box2 = Gtk::DropTarget::create(Glib::Value<Gtk::Button*>::value_type(), Gdk::DragAction::MOVE);
        target_drop_controller_box2->signal_drop().connect(
            [this](const Glib::ValueBase& value, double x, double y) {
                std::cout << "Fatal, it's not working for some reason" << std::endl;
                if (G_VALUE_HOLDS(value.gobj(), Glib::Value<Gtk::Button*>::value_type())) {
                    Glib::Value<Gtk::Button*> button;
                    button.init(value.gobj());
                    auto transferred_button = button.get();
                    // TODO: I'll have to check if this button isn't in the listbox already
                    box1.remove(*transferred_button);
                    box2.append(*transferred_button);
                    std::cout << "Button added successfully" << std::endl;
                } else {
                    std::cout << "Wtf???" << std::endl;
                }
                return true;
            }
            ,false);
        box2.add_controller(target_drop_controller_box2);
        box2.set_expand();
        box2.add_css_class("rich-list");
        box2.add_css_class("boxed-list");
        root_box->append(box2);
    }

protected:
    std::vector<Gtk::Button*> in_box1, in_box2;
    Gtk::Box box1, box2;
    Gtk::Button add_item_button;

    void on_add_item() {
        auto new_item = Gtk::make_managed<Gtk::Button>("I'm a button lmao");
        // Sets the button itself as the content to be drag to another box
        auto drag_source_controller = Gtk::DragSource::create();
        drag_source_controller->set_actions(Gdk::DragAction::MOVE);
        Glib::Value<Gtk::Button*> value;
        value.init(value.value_type());
        value.set(new_item);
        auto content_provider = Gdk::ContentProvider::create(value);
        drag_source_controller->set_content(content_provider);
        new_item->add_controller(drag_source_controller);
        box1.append(*new_item);
    }
};

int main(int argv, char** argc) {
    auto app = Gtk::Application::create("com.moura.dnd");
    return app->make_window_and_run<AppWindow>(argv, argc);
}