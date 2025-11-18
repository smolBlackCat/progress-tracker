#include "task-widget.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
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

TaskWidget::TaskWidget(CardDetailsDialog& card_details_dialog,
                       const std::shared_ptr<Task>& task, bool is_new)
    : Glib::ObjectBase{"TaskWidget"},
      TaskInit{},
      BaseItem{Gtk::Orientation::HORIZONTAL, 3},
      m_card_dialog{card_details_dialog},
      m_card_widget{card_details_dialog.get_card_widget()},
      m_task{task},
      m_focus_controller{Gtk::EventControllerFocus::create()},
      m_menu_model{Gio::Menu::create()},
      m_action_group{Gio::SimpleActionGroup::create()},
      m_is_new{is_new} {
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
    m_label.set_label(task->get_name());

    inner_box.append(m_entry_revealer);
    m_entry_revealer.set_hexpand();
    m_entry_revealer.set_child(m_entry);

    inner_box.insert_at_end(*this);

    m_checkbutton.insert_at_end(*this);
    m_checkbutton.set_halign(Gtk::Align::END);
    m_checkbutton.set_hexpand();
    m_checkbutton.set_margin_end(5);

    if (task->get_done()) add_css_class("complete-task");

    m_checkbutton.signal_toggled().connect(
        sigc::mem_fun(*this, &TaskWidget::on_checkbox));
    m_checkbutton.set_active(task->get_done());

    m_menu_model->append(_("Rename"), "task-widget.rename");
    m_menu_model->append(_("Remove"), "task-widget.remove");
    m_menu_model->append(_("Convert to card"), "task-widget.task-convert");
    m_action_group->add_action("rename",
                               sigc::mem_fun(*this, &TaskWidget::on_rename));
    m_action_group->add_action("remove",
                               sigc::mem_fun(*this, &TaskWidget::on_remove));
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
    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (m_entry_revealer.get_child_revealed()) {
                switch (keyval) {
                    case (GDK_KEY_Return): {
                        off_rename();
                        break;
                    }
                }
            }
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
              auto new_task = card_details_dialog.insert_new_task_after(
                  Task{_("New Task")}, this);
              new_task->grab_focus();
              return true;
          }},
         {"<Control>Delete",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              on_remove();
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
                  this->m_card_dialog.reorder_task_widget(prev_task, *this);
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
                  this->m_card_dialog.reorder_task_widget(*this, next_task);
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

    setup_drag_and_drop();

    if (is_new) {
        on_rename();
    }
}

std::shared_ptr<Task> TaskWidget::task() const { return m_task; }

void TaskWidget::on_rename() {
    m_entry.remove_controller(m_focus_controller);
    m_entry_revealer.set_reveal_child();
    m_label.set_visible(false);

    m_entry.set_text(m_task->get_name());
    m_entry.grab_focus();

    m_entry.add_controller(m_focus_controller);

    spdlog::get("ui")->debug(
        "[TaskWidget.on_rename] (\"{}\"): Entered rename mode",
        m_task->get_name());
}

void TaskWidget::off_rename() {
    m_entry_revealer.set_reveal_child(false);
    m_label.set_visible(true);

    std::string new_label = m_entry.get_text();
    if (new_label != m_task->get_name()) {
        if (m_checkbutton.get_active()) {
            m_label.set_markup(Glib::ustring::compose("<s>%1</s>", new_label));
        } else {
            m_label.set_markup(new_label);
        }
        const std::string old_name = m_task->get_name();
        m_task->set_name(new_label);

        spdlog::get("app")->info(
            "Task (\"{}\") from Card (\"{}\") renamed to (\"{}\")", old_name,
            m_card_widget->get_card()->get_name(), new_label);
        m_is_new = false;
    }

    spdlog::get("ui")->debug(
        "[TaskWidget.off_rename] (\"{}\"): Exited rename mode",
        m_task->get_name());
}

void TaskWidget::on_remove() { m_card_dialog.remove_task(*this); }

void TaskWidget::on_checkbox() {
    bool done = m_checkbutton.get_active();
    this->m_task->set_done(done);

    if (done) {
        m_label.set_markup(
            Glib::ustring::compose("<s>%1</s>", m_task->get_name()));
        add_css_class("complete-task");
    } else {
        m_label.set_markup(m_task->get_name());
        remove_css_class("complete-task");
    }

    spdlog::get("app")->info("Task (\"{}\") from Card (\"{}\") set as {}",
                             m_task->get_name(),
                             m_card_widget->get_card()->get_name(),
                             (done ? "complete" : "incomplete"));

    m_card_widget->update_complete_tasks_label();
}

void TaskWidget::on_convert() {
    spdlog::get("app")->info(
        "Task (\"{}\") from Card (\"{}\") converted into a card",
        m_task->get_name(), m_card_widget->get_card()->get_name());
    auto cardlist_widget =
        const_cast<CardlistWidget*>(m_card_widget->get_cardlist_widget());
    auto task_as_card =
        cardlist_widget->append_new_card(Card{m_task->get_name()});

    cardlist_widget->reorder(*task_as_card, *m_card_widget);
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
        [this](const Glib::RefPtr<Gdk::Drag>& drag) {
            this->set_opacity(0.5);
            spdlog::get("ui")->debug("[TaskWidget.dnd] (\"{}\"): On drag",
                                     m_task->get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            spdlog::get("ui")->debug(
                "[TaskWidget.dnd] (\"{}\"): Canceled drag event",
                m_task->get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
            this->set_opacity(1);
            spdlog::get("ui")->debug(
                "[TaskWidget.dnd] (\"{}\"): Stopped being dragged",
                m_task->get_name());
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
                        "[TaskWidget.dnd] (\"{}\") has been dropped onto "
                        "itself",
                        m_task->get_name());
                    return true;
                }

                spdlog::get("app")->info(
                    "Task (\"{}\") dropped on Task (\"{}\")",
                    dropped_taskwidget->task()->get_name(), m_task->get_name());

                m_card_dialog.reorder_task_widget(*dropped_taskwidget, *this);

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
