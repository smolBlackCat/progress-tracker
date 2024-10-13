#include "task-widget.h"

#include <glibmm/i18n.h>
#include <widgets/card.h>
#include <widgets/cardlist-widget.h>

#include <memory>

#include "dialog/card-dialog.h"
#include "gdk/gdkkeysyms.h"
#include "gdkmm/rectangle.h"
#include "glibmm/ustring.h"
#include "sigc++/functors/mem_fun.h"

namespace ui {

TaskWidget::TaskWidget(BaseObjectType* cobject,
                       const Glib::RefPtr<Gtk::Builder>& builder,
                       CardDetailsDialog& card_details_dialog,
                       CardWidget& card_widget, std::shared_ptr<Task> task,
                       bool is_new)
    : Gtk::Box{cobject},
      card_details_dialog{card_details_dialog},
      task{task},
      menu_model{Gio::Menu::create()},
      group{Gio::SimpleActionGroup::create()},
      popover_menu{menu_model},
      is_new{is_new} {
    if (task->get_done()) {
        add_css_class("complete-task");
    } else {
        add_css_class("incomplete-task");
    }

    task_label = builder->get_widget<Gtk::Label>("task-label");
    task_label->set_label(task->get_name());
    task_entry_revealer =
        builder->get_widget<Gtk::Revealer>("task-entry-revealer");
    task_entry = builder->get_widget<Gtk::Entry>("task-entry");
    task_checkbutton =
        builder->get_widget<Gtk::CheckButton>("task-checkbutton");

    task_checkbutton->signal_toggled().connect(
        sigc::mem_fun(*this, &TaskWidget::on_checkbox));
    task_checkbutton->set_active(task->get_done());

    menu_model->append(_("Rename"), "task-widget.rename");
    menu_model->append(_("Remove"), "task-widget.remove");
    menu_model->append(_("Convert to card"), "task-widget.task-convert");
    group->add_action("rename", sigc::mem_fun(*this, &TaskWidget::on_rename));
    group->add_action("remove", sigc::mem_fun(*this, &TaskWidget::on_remove));
    group->add_action("task-convert", [this, &card_widget]() {
        this->on_convert(card_widget);
    });
    popover_menu.insert_action_group("task-widget", group);
    popover_menu.set_parent(*this);

    auto gesture_click = Gtk::GestureClick::create();
    gesture_click->set_button();
    gesture_click->signal_released().connect(
        [this, gesture_click](int n_pressed, double x, double y) {
            if (n_pressed >= 1 &&
                gesture_click->get_current_button() == GDK_BUTTON_PRIMARY) {
                on_rename();
            } else if (n_pressed == 1 && gesture_click->get_current_button() ==
                                             GDK_BUTTON_SECONDARY) {
                this->popover_menu.set_pointing_to(Gdk::Rectangle(x, y, 0, 0));
                popover_menu.popup();
            }
        });
    this->add_controller(gesture_click);

    auto focus_controller = Gtk::EventControllerFocus::create();
    focus_controller->signal_leave().connect(
        sigc::mem_fun(*this, &TaskWidget::off_rename));
    task_entry->add_controller(focus_controller);

    auto key_controller = Gtk::EventControllerKey::create();
    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (task_entry_revealer->get_child_revealed()) {
                switch (keyval) {
                    case (GDK_KEY_Return): {
                        off_rename();
                        break;
                    }
                }
            }
        },
        false);
    task_entry->add_controller(key_controller);

    setup_drag_and_drop();

    if (is_new) {
        on_rename();
    }
}

TaskWidget::~TaskWidget() {}

std::shared_ptr<Task> TaskWidget::get_task() { return task; }

void TaskWidget::on_rename() {
    task_entry_revealer->set_reveal_child();
    task_label->set_visible(false);

    task_entry->set_text(task->get_name());
}

void TaskWidget::off_rename() {
    task_entry_revealer->set_reveal_child(false);
    task_label->set_visible(true);

    std::string new_label = task_entry->get_text();
    if (new_label != task->get_name()) {
        if (task_checkbutton->get_active()) {
            task_label->set_markup(
                Glib::ustring::compose("<s>%1</s>", new_label));
        } else {
            task_label->set_markup(new_label);
        }
        task->set_name(new_label);
        is_new = false;
    }
}

void TaskWidget::on_remove() { card_details_dialog.remove_task(*this); }

void TaskWidget::on_checkbox() {
    this->task->set_done(task_checkbutton->get_active());
    this->card_details_dialog.get_card_widget().update_complete_tasks();
    if (task_checkbutton->get_active()) {
        task_label->set_markup(
            Glib::ustring::compose("<s>%1</s>", task->get_name()));
        add_css_class("complete-task");
        remove_css_class("incomplete-task");
    } else {
        task_label->set_markup(task->get_name());
        add_css_class("incomplete-task");
        remove_css_class("complete-task");
    }
}

void TaskWidget::on_convert(CardWidget& card_widget) {
    auto cardlist_widget =
        const_cast<CardlistWidget*>(card_widget.get_cardlist_widget());
    auto cardlist_model = cardlist_widget->get_cardlist();
    auto task_as_card = cardlist_widget->add_card(Card{task->get_name()});

    cardlist_widget->reorder_cardwidget(*task_as_card, card_widget);
    card_details_dialog.remove_task(*this);
}

void TaskWidget::setup_drag_and_drop() {
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->signal_prepare().connect(
        [this, drag_source_c](double x, double y) {
            Glib::Value<ui::TaskWidget*> value_taskwidget_p;
            value_taskwidget_p.init(Glib::Value<ui::TaskWidget*>::value_type());
            value_taskwidget_p.set(this);
            auto taskwidget_icon = Gtk::WidgetPaintable::create(*this);
            drag_source_c->set_icon(taskwidget_icon, x, y);
            return Gdk::ContentProvider::create(value_taskwidget_p);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag) { this->set_opacity(0.5); },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
            this->set_opacity(1);
        });
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    add_controller(drag_source_c);

    auto drop_target_taskwidget = Gtk::DropTarget::create(
        Glib::Value<ui::TaskWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_taskwidget->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::TaskWidget*>::value_type())) {
                Glib::Value<ui::TaskWidget*> dropped_value;
                dropped_value.init(value.gobj());

                ui::TaskWidget* dropped_taskwidget = dropped_value.get();

                if (dropped_taskwidget == this) {
                    return true;
                }

                card_details_dialog.reorder_task_widget(*dropped_taskwidget,
                                                        *this);

                dropped_taskwidget->set_opacity(1);
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_taskwidget);
}
}  // namespace ui
