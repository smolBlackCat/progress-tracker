#include "task-widget.h"

#include <glibmm/i18n.h>
#include <widgets/card-widget.h>
#include <widgets/cardlist-widget.h>

#include <memory>

#include "dialog/card-dialog.h"

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

TaskWidget::TaskWidget(CardDialog& card_details_dialog, const std::string& name,
                       bool complete)
    : Glib::ObjectBase{"TaskWidget"},
      TaskInit{},
      BaseItem{Gtk::Orientation::HORIZONTAL, 3},
      m_card_dialog{card_details_dialog},
      m_card_widget{card_details_dialog.card_widget()},
      m_focus_controller{Gtk::EventControllerFocus::create()},
      m_menu_model{Gio::Menu::create()},
      m_action_group{Gio::SimpleActionGroup::create()},
      m_title{name} {
    m_popover.set_menu_model(m_menu_model);
    set_margin_start(5);
    set_margin_end(5);

    Gtk::Box& inner_box =
        *Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    inner_box.set_valign(Gtk::Align::CENTER);
    inner_box.append(m_label);
    m_label.set_halign(Gtk::Align::START);
    m_label.set_margin_start(5);
    m_label.set_wrap();
    m_label.set_wrap_mode(Pango::WrapMode::WORD_CHAR);

    if (complete) {
        add_css_class("complete-task");
        m_label.set_markup(std::format("<s>{}</s>", name));
    } else {
        m_label.set_label(name);
    }

    inner_box.append(m_entry_revealer);
    m_entry_revealer.set_hexpand();
    m_entry_revealer.set_child(m_entry);

    inner_box.insert_at_end(*this);

    m_checkbutton.insert_at_end(*this);
    m_checkbutton.set_halign(Gtk::Align::END);
    m_checkbutton.set_hexpand();
    m_checkbutton.set_margin_end(5);
    m_checkbutton.set_can_focus(false);

    m_checkbutton.set_active(complete);
    m_checkbutton.signal_toggled().connect(
        sigc::mem_fun(*this, &TaskWidget::on_checkbox));

    m_menu_model->append(_("Rename"), "task-widget.rename");
    m_menu_model->append(_("Remove"), "task-widget.remove");
    m_menu_model->append(_("Convert to card"), "task-widget.task-convert");
    m_action_group->add_action("rename",
                               sigc::mem_fun(*this, &TaskWidget::on_rename));
    m_action_group->add_action("remove",
                               [&]() { m_card_dialog.remove_task(*this); });
    m_action_group->add_action("task-convert",
                               sigc::mem_fun(*this, &TaskWidget::on_convert));
    m_popover.insert_action_group("task-widget", m_action_group);
    m_popover.set_parent(*this);

    auto gesture_click = Gtk::GestureClick::create();
    gesture_click->set_button();
    gesture_click->signal_released().connect(
        [this, gesture_click](int n_pressed, double x, double y) {
            if (n_pressed >= 1 &&
                gesture_click->get_current_button() == GDK_BUTTON_PRIMARY) {
                on_rename();
            } else if (n_pressed == 1 && gesture_click->get_current_button() ==
                                             GDK_BUTTON_SECONDARY) {
                this->m_popover.set_pointing_to(Gdk::Rectangle(x, y, 0, 0));
                m_popover.popup();
            }
        });
    this->add_controller(gesture_click);

    auto key_controller = Gtk::EventControllerKey::create();
    key_controller->signal_key_pressed().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (m_entry_revealer.get_child_revealed()) {
                switch (keyval) {
                    case (GDK_KEY_Escape): {
                        // TODO: Extract this behaviour into a toggle method
                        m_entry_revealer.set_reveal_child(false);
                        m_label.set_visible();
                        return true;
                    }
                    case (GDK_KEY_Return):
                    case (GDK_KEY_KP_Enter): {
                        off_rename();
                        return true;
                    }
                }
            }
            return false;
        },
        false);
    m_focus_controller->signal_leave().connect(
        sigc::mem_fun(*this, &TaskWidget::off_rename));

    m_entry.add_controller(key_controller);
    m_entry.add_controller(m_focus_controller);

    using TaskShortcut =
        std::pair<const char*,
                  std::function<bool(Gtk::Widget&, const Glib::VariantBase&)>>;

    const std::array<TaskShortcut, 7> task_shortcuts = {
        {{"<Control>R",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              on_rename();
              return true;
          }},
         {"<Control>N",
          [&card_details_dialog, this](Gtk::Widget&, const Glib::VariantBase&) {
              auto new_task =
                  Gtk::make_managed<TaskWidget>(m_card_dialog, _("New Task"));
              card_details_dialog.insert_after(*new_task, *this);
              new_task->grab_focus();
              return true;
          }},
         {"<Control>Delete",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              m_card_dialog.remove_task(*this);
              return true;
          }},
         {"<Control><Shift>C",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              on_convert();
              return true;
          }},
         {"<Control>Up",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              Gtk::Widget* previous = this->get_prev_sibling();
              if (previous) {
                  TaskWidget& prev_task = *static_cast<TaskWidget*>(previous);
                  this->m_card_dialog.reorder(prev_task, *this);
              } else {
                  this->error_bell();
              }
              return true;
          }},
         {"<Control>Down",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              Gtk::Widget* next = this->get_next_sibling();
              if (!G_TYPE_CHECK_INSTANCE_TYPE(next->gobj(),
                                              Gtk::Button::get_type())) {
                  TaskWidget& next_task = *static_cast<TaskWidget*>(next);
                  this->m_card_dialog.reorder(*this, next_task);
              } else {
                  this->error_bell();
              }
              return true;
          }},
         {"Menu|<Shift>F10", [this](Gtk::Widget&, const Glib::VariantBase&) {
              m_popover.popup();
              return true;
          }}}};

    auto shortcut_controller = Gtk::ShortcutController::create();
    shortcut_controller->set_scope(Gtk::ShortcutScope::LOCAL);

    for (const auto& [keybinding, callback] : task_shortcuts) {
        shortcut_controller->add_shortcut(Gtk::Shortcut::create(
            Gtk::ShortcutTrigger::parse_string(keybinding),
            Gtk::CallbackAction::create(callback)));
    }

    add_controller(shortcut_controller);

    auto key_controller2 = Gtk::EventControllerKey::create();
    key_controller2->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType modifier) {
            if (this->has_focus()) {
                switch (keyval) {
                    case (GDK_KEY_space):
                    case (GDK_KEY_Return):
                    case (GDK_KEY_KP_Enter): {
                        m_checkbutton.set_active(!m_checkbutton.get_active());
                        break;
                    }
                }
            }
        });
    add_controller(key_controller2);

    setup_drag_and_drop();
}

void TaskWidget::set_title(const std::string& title) {
    if (!title.empty()) {
        const std::string old_title = m_title;
        m_title = title;

        m_label.set_markup(m_checkbutton.get_active()
                               ? std::format("<s>{}</s>", m_title)
                               : m_title);
        m_name_changed_signal.emit(old_title, title);
    }
}

void TaskWidget::set_complete(bool complete) {
    m_checkbutton.set_active(complete);
}

std::string TaskWidget::get_title() const { return m_title; }

bool TaskWidget::get_complete() const { return m_checkbutton.get_active(); }

sigc::signal<void(std::string, std::string)>&
TaskWidget::signal_name_changed() {
    return m_name_changed_signal;
}

sigc::signal<void()>& TaskWidget::signal_complete_changed() {
    return m_complete_changed_signal;
}

void TaskWidget::on_rename() {
    m_entry.remove_controller(m_focus_controller);
    m_entry_revealer.set_reveal_child();
    m_label.set_visible(false);

    m_entry.set_text(m_title);
    m_entry.grab_focus();

    m_entry.add_controller(m_focus_controller);
}

void TaskWidget::off_rename() {
    m_entry_revealer.set_reveal_child(false);
    m_label.set_visible(true);

    const std::string new_label = m_entry.get_text();
    const std::string old_label = m_title;
    if (new_label != old_label) {
        if (m_checkbutton.get_active()) {
            m_label.set_markup(Glib::ustring::compose("<s>%1</s>", new_label));
        } else {
            m_label.set_markup(new_label);
        }
        m_title = new_label;
        m_name_changed_signal.emit(old_label, new_label);
    }
}

void TaskWidget::on_checkbox() {
    m_complete_changed_signal.emit();

    if (m_checkbutton.get_active()) {
        m_label.set_markup(Glib::ustring::compose("<s>%1</s>", m_title));
        add_css_class("complete-task");
    } else {
        m_label.set_label(m_title);
        remove_css_class("complete-task");
    }
}

void TaskWidget::on_convert() {
    auto cardlist_widget = const_cast<CardlistWidget*>(m_card_widget->parent());
    auto task_as_card = Gtk::make_managed<CardWidget>(m_title);

    cardlist_widget->insert_after(*task_as_card, *m_card_widget);
    m_card_dialog.remove_task(*this);
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

                m_card_dialog.reorder(*dropped_taskwidget, *this);

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
    m_checkbutton.unparent();
    m_popover.unparent();
}
}  // namespace ui
