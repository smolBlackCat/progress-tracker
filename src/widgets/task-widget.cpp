#include "task-widget.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <widgets/card-widget.h>
#include <widgets/cardlist-widget.h>

#include <memory>

#include "dialog/card-dialog.h"
#include "gtkmm/shortcutaction.h"

extern "C" {
static void task_class_init(void* klass, void* user_data) {
    g_return_if_fail(GTK_IS_WIDGET_CLASS(klass));
    gtk_widget_class_set_css_name(GTK_WIDGET_CLASS(klass), "task");
}

static void task_init(GTypeInstance* instance, void* klass) {
    g_return_if_fail(GTK_IS_WIDGET(instance));

    gtk_widget_set_receives_default(GTK_WIDGET(instance), TRUE);
    gtk_widget_set_focusable(GTK_WIDGET(instance), TRUE);
}
}

namespace ui {

TaskInit::TaskInit()
    : Glib::ExtraClassInit{task_class_init, nullptr, task_init} {}

TaskWidget::TaskWidget(CardDetailsDialog& card_details_dialog,
                       CardWidget& card_widget, std::shared_ptr<Task> task,
                       bool is_new)
    : Glib::ObjectBase{"TaskWidget"},
      TaskInit{},
      BaseItem{Gtk::Orientation::HORIZONTAL, 3},
      card_details_dialog{card_details_dialog},
      task{task},
      menu_model{Gio::Menu::create()},
      group{Gio::SimpleActionGroup::create()},
      popover_menu{menu_model},
      is_new{is_new} {
    set_margin_start(5);
    set_margin_end(5);

    Gtk::Box& inner_box =
        *Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    inner_box.set_valign(Gtk::Align::CENTER);
    inner_box.append(task_label);
    task_label.set_halign(Gtk::Align::START);
    task_label.set_margin_start(5);
    task_label.set_wrap();
    task_label.set_wrap_mode(Pango::WrapMode::WORD_CHAR);
    task_label.set_label(task->get_name());

    inner_box.append(task_entry_revealer);
    task_entry_revealer.set_hexpand();
    task_entry_revealer.set_child(task_entry);

    inner_box.insert_at_end(*this);

    task_checkbutton.insert_at_end(*this);
    task_checkbutton.set_halign(Gtk::Align::END);
    task_checkbutton.set_hexpand();
    task_checkbutton.set_margin_end(5);

    if (task->get_done()) add_css_class("complete-task");

    task_checkbutton.signal_toggled().connect(
        sigc::mem_fun(*this, &TaskWidget::on_checkbox));
    task_checkbutton.set_active(task->get_done());

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

    auto key_controller = Gtk::EventControllerKey::create();
    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (task_entry_revealer.get_child_revealed()) {
                switch (keyval) {
                    case (GDK_KEY_Return): {
                        off_rename();
                        break;
                    }
                }
            }
        },
        false);
    task_entry.add_controller(key_controller);
    auto focus_controller = Gtk::EventControllerFocus::create();
    focus_controller->signal_leave().connect(
        sigc::mem_fun(*this, &TaskWidget::off_rename));
    task_entry.add_controller(focus_controller);

    auto shortcut_controller = Gtk::ShortcutController::create();
    shortcut_controller->set_scope(Gtk::ShortcutScope::LOCAL);
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>R"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                on_rename();
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Delete"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                on_remove();
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control><Shift>C"),
        Gtk::CallbackAction::create(
            [this, &card_widget](Gtk::Widget&, const Glib::VariantBase&) {
                on_convert(card_widget);
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Up"),
        Gtk::CallbackAction::create([this](Gtk::Widget&,
                                           const Glib::VariantBase&) {
            Gtk::Widget* previous = this->get_prev_sibling();
            if (previous) {
                TaskWidget& prev_task = *static_cast<TaskWidget*>(previous);
                this->card_details_dialog.reorder_task_widget(prev_task, *this);
            }
            return true;
        })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Down"),
        Gtk::CallbackAction::create([this](Gtk::Widget&,
                                           const Glib::VariantBase&) {
            Gtk::Widget* next = this->get_next_sibling();
            if (!G_TYPE_CHECK_INSTANCE_TYPE(next->gobj(),
                                            Gtk::Button::get_type())) {
                TaskWidget& next_task = *static_cast<TaskWidget*>(next);
                this->card_details_dialog.reorder_task_widget(*this, next_task);
            }
            return true;
        })));
    add_controller(shortcut_controller);

    setup_drag_and_drop();

    if (is_new) {
        on_rename();
    }
}

std::shared_ptr<Task> TaskWidget::get_task() { return task; }

void TaskWidget::on_rename() {
    task_entry_revealer.set_reveal_child();
    task_label.set_visible(false);

    task_entry.set_text(task->get_name());
    task_entry.grab_focus();

    spdlog::get("ui")->debug("TaskWidget \"{}\" has entered rename mode",
                             task->get_name());
}

void TaskWidget::off_rename() {
    task_entry_revealer.set_reveal_child(false);
    task_label.set_visible(true);

    std::string new_label = task_entry.get_text();
    if (new_label != task->get_name()) {
        if (task_checkbutton.get_active()) {
            task_label.set_markup(
                Glib::ustring::compose("<s>%1</s>", new_label));
        } else {
            task_label.set_markup(new_label);
        }
        task->set_name(new_label);
        is_new = false;
    }

    spdlog::get("ui")->debug("TaskWidget \"{}\" has exited rename mode",
                             task->get_name());
}

void TaskWidget::on_remove() { card_details_dialog.remove_task(*this); }

void TaskWidget::on_checkbox() {
    this->task->set_done(task_checkbutton.get_active());
    this->card_details_dialog.get_card_widget()->update_complete_tasks();
    if (task_checkbutton.get_active()) {
        task_label.set_markup(
            Glib::ustring::compose("<s>%1</s>", task->get_name()));
        add_css_class("complete-task");

        spdlog::get("ui")->debug(
            "TaskWidget \"{}\" has been marked as complete", task->get_name());
    } else {
        task_label.set_markup(task->get_name());
        remove_css_class("complete-task");

        spdlog::get("ui")->debug(
            "TaskWidget \"{}\" has been marked as incomplete",
            task->get_name());
    }
}

void TaskWidget::on_convert(CardWidget& card_widget) {
    spdlog::get("app")->info("Task \"{}\" has been converted to a card",
                             task->get_name());
    auto cardlist_widget =
        const_cast<CardlistWidget*>(card_widget.get_cardlist_widget());
    auto cardlist_model = cardlist_widget->get_cardlist();
    auto task_as_card = cardlist_widget->add(Card{task->get_name()});

    cardlist_widget->reorder(*task_as_card, card_widget);
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
        [this](const Glib::RefPtr<Gdk::Drag>& drag) {
            this->set_opacity(0.5);
            spdlog::get("ui")->debug("TaskWidget \"{}\" has started dragging",
                                     task->get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            spdlog::get("ui")->debug("TaskWidget \"{}\" has cancelled dragging",
                                     task->get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
            this->set_opacity(1);
            spdlog::get("ui")->debug("TaskWidget \"{}\" has ended dragging",
                                     task->get_name());
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
                    spdlog::get("ui")->warn(
                        "Dropped TaskWidget \"{}\" onto itself",
                        task->get_name());
                    return true;
                }

                spdlog::get("ui")->debug(
                    "TaskWidget \"{}\" has been dropped on TaskWidget \"{}\"",
                    dropped_taskwidget->get_task()->get_name(),
                    task->get_name());

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

void TaskWidget::cleanup() {
    static_cast<Gtk::Box*>(get_first_child())->unparent();  // inner_box
    task_checkbutton.unparent();
    popover_menu.unparent();
}
}  // namespace ui
