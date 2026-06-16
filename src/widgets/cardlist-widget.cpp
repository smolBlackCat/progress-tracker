#include "cardlist-widget.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>

#include "board-widget.h"
#include "card-widget.h"

extern "C" {
static void cardlist_class_init(void* klass, void* user_data) {
    g_return_if_fail(GTK_IS_WIDGET_CLASS(klass));
    gtk_widget_class_set_css_name(GTK_WIDGET_CLASS(klass), "cardlist");
}

static void cardlist_init(GTypeInstance* instance, void* klass) {
    g_return_if_fail(GTK_IS_WIDGET(instance));

    gtk_widget_set_focusable(GTK_WIDGET(instance), TRUE);
    gtk_widget_set_receives_default(GTK_WIDGET(instance), TRUE);
}
}

namespace ui {

struct CardlistPayload {
    CardlistWidget* cardlist_widget;
};

CardlistInit::CardlistInit()
    : Glib::ExtraClassInit(cardlist_class_init, nullptr, cardlist_init) {}

CardlistWidget::CardlistWidget(BoardWidget& board, const std::string& name)
    : Glib::ObjectBase{"CardlistWidget"},
      CardlistInit{},
      BaseItem{Gtk::Orientation::VERTICAL, 0},
      m_add_card_button{_("Add card")},
      m_root{Gtk::Orientation::VERTICAL},
      board{board},
      m_header{name, "title-2", "title-2"},
      m_name{name},
      m_scr_window{} {
    set_layout_manager(Gtk::BoxLayout::create(Gtk::Orientation::VERTICAL));
    add_css_class("cardlist");
    set_halign(Gtk::Align::START);
    set_size_request(CARDLIST_MAX_WIDTH, -1);
    setup_drag_and_drop();

    m_header.add_option_button(_("Remove"), "remove",
                               [this]() { this->board.remove(*this); });
    m_header.signal_on_confirm().connect(
        sigc::mem_fun(*this, &CardlistWidget::set_title));

    // FIXME: We're getting EditableLabelHeader instance's menus, but honestly,
    // that is quite dirty, primarily when the other widgets implement their
    // header as well rather than having a helper widget
    m_popover.set_menu_model(m_header.get_menu_model());
    m_popover.insert_action_group("label-header", m_header.get_menu_actions());
    m_popover.set_has_arrow(false);
    m_popover.set_parent(m_header);
    m_header.set_margin_bottom(15);

    m_add_card_button.set_valign(Gtk::Align::CENTER);
    m_add_card_button.set_hexpand(true);
    m_add_card_button.signal_clicked().connect([this]() {
        CardWidget* card = Gtk::make_managed<CardWidget>(_("New Card"));
        append(*card);
    });
    m_root.append(m_add_card_button);

    m_header.insert_at_start(*this);

    m_root.set_vexpand();
    m_root.set_spacing(15);

    m_scr_window.set_child(m_root);
    m_scr_window.set_size_request(CARDLIST_MAX_WIDTH, -1);
    m_scr_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    m_scr_window.insert_at_end(*this);
    using CardListShortcut =
        std::pair<const char*,
                  std::function<bool(Gtk::Widget&, const Glib::VariantBase&)>>;

    const std::array<CardListShortcut, 6> cardlist_shortcuts = {
        {{"<Control>R",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              this->m_header.to_editing_mode();
              return true;
          }},
         {"<Control>N",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              CardlistWidget* cardlist_widget =
                  Gtk::make_managed<CardlistWidget>(this->board,
                                                    _("New Cardlist"));
              this->board.insert_after(*cardlist_widget, *this);
              cardlist_widget->grab_focus();

              return true;
          }},
         {"<Control>Delete",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              this->board.remove(*this);
              return true;
          }},
         {"<Control>Left",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              CardlistWidget* previous_cardlist =
                  static_cast<CardlistWidget*>(this->get_prev_sibling());
              if (previous_cardlist) {
                  this->board.reorder(*previous_cardlist, *this);
              } else {
                  this->error_bell();
              }
              return true;
          }},
         {"<Control>Right",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              Widget* maybe_cardlist = this->get_next_sibling();
              if (!G_TYPE_CHECK_INSTANCE_TYPE(maybe_cardlist->gobj(),
                                              Gtk::Button::get_type())) {
                  CardlistWidget* cardlist_widget =
                      static_cast<CardlistWidget*>(maybe_cardlist);
                  this->board.reorder(*this, *cardlist_widget);
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

    for (const auto& [keybinding, callback] : cardlist_shortcuts) {
        shortcut_controller->add_shortcut(Gtk::Shortcut::create(
            Gtk::ShortcutTrigger::parse_string(keybinding),
            Gtk::CallbackAction::create(callback)));
    }

    add_controller(shortcut_controller);

    auto gesture_click = Gtk::GestureClick::create();

    gesture_click->set_button(0);
    gesture_click->signal_released().connect(
        [this, gesture_click](int n_pressed, double x, double y) {
            if (n_pressed == 1 &&
                gesture_click->get_current_button() == GDK_BUTTON_SECONDARY) {
                this->m_popover.set_pointing_to(Gdk::Rectangle(x, y, 0, 0));
                m_popover.popup();
            }
        });
    add_controller(gesture_click);
}

void CardlistWidget::set_title(const std::string& name) {
    std::string old_name = m_name;
    m_name = name;

    m_name_changed_signal.emit(old_name, m_name);
}

void CardlistWidget::reorder(CardWidget& next, CardWidget& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    ssize_t c_i = 0;
    for (Gtk::Widget* cur_card = m_root.get_first_child(); cur_card;
         cur_card = cur_card->get_next_sibling()) {
        if (&next == cur_card) {
            next_i = c_i;
        } else if (&sibling == cur_card) {
            sibling_i = c_i;
        }
        c_i++;
    }

    if ((next_i == -1) || (sibling_i == -1)) {
        return;
    }

    bool up = false;
    if (sibling.get_prev_sibling() == &next) {
        m_root.reorder_child_after(next, sibling);
    } else if (sibling.get_next_sibling() == &next) {
        m_root.reorder_child_after(sibling, next);
        up = true;
    } else {
        // Widgets are not neighbours. How to reorder them now depends on their
        // index
        if (next_i > sibling_i) {  // Move the widget up
            sibling.get_prev_sibling() == nullptr
                ? m_root.reorder_child_at_start(next)
                : m_root.reorder_child_after(next, *sibling.get_prev_sibling());
                up = true;
        } else {  // Move the widget down
            m_root.reorder_child_after(next, sibling);
        }
    }

    m_card_reorder_signal.emit(&next, &sibling, up);
}

void CardlistWidget::remove(CardWidget& card) {
    spdlog::get("app")->info("Card (\"{}\") removed from CardList (\"{}\")",
                             card.get_title(), get_name());

    // Focus should be passed to the immediate sibling
    Gtk::Widget* next_card = card.get_next_sibling();
    Gtk::Widget* prev_card = card.get_prev_sibling();

    if (prev_card) {
        prev_card->grab_focus();
    } else if (next_card && !G_TYPE_CHECK_INSTANCE_TYPE(
                                next_card->gobj(), Gtk::Button::get_type())) {
        next_card->grab_focus();
    }

    m_card_remove_signal.emit(&card);
    m_root.remove(card);
}

void CardlistWidget::append(CardWidget& card) {
    if (!card.parent()) {
        card.set_cardlist(this);
        m_root.append(card);
        m_root.reorder_child_after(m_add_card_button, card);

        m_card_add_signal.emit(&card, -1);
    }
}

void CardlistWidget::insert_after(CardWidget& card, CardWidget& sibling) {
    if (!card.parent() && sibling.parent() == this) {
        ssize_t index = 0;
        for (Gtk::Widget* card = m_root.get_first_child(); card != &sibling;
             card = card->get_next_sibling())
            index++;
        m_root.insert_child_after(card, sibling);
        card.set_cardlist(this);

        m_card_add_signal.emit(&card, index);
    }
}

void CardlistWidget::receive(CardWidget& card) {
    if (card.parent()) {
        CardlistWidget* old_parent = card.parent();
        card.reference();
        card.parent()->signal_card_removed().block();
        card.remove_from_parent();

        card.set_cardlist(this);
        m_root.append(card);
        m_root.reorder_child_after(m_add_card_button, card);

        card.parent()->signal_card_removed().unblock();
        card.unreference();

        m_card_received_signal.emit(&card, old_parent, nullptr);
    }
}

void CardlistWidget::receive_after(CardWidget& card, CardWidget& sibling) {
    if (card.parent() && sibling.parent() == this) {
        CardlistWidget* old_parent = card.parent();
        card.reference();
        card.parent()->signal_card_removed().block();

        card.remove_from_parent();
        card.set_cardlist(this);
        m_root.append(card);
        m_root.reorder_child_after(card, sibling);

        card.parent()->signal_card_removed().unblock();
        card.unreference();

        m_card_received_signal.emit(&card, old_parent, &sibling);
    }
}

const std::string& CardlistWidget::get_name() const { return m_name; }

bool CardlistWidget::is_child(CardWidget& card) {
    for (Gtk::Widget* cur_card = m_root.get_first_child(); cur_card;
         cur_card = cur_card->get_next_sibling()) {
        if (cur_card == &card) {
            return true;
        }
    }
    return false;
}

sigc::signal<void(std::string, std::string)>&
CardlistWidget::signal_name_changed() {
    return m_name_changed_signal;
}

sigc::signal<void(CardWidget*, int)>& CardlistWidget::signal_card_added() {
    return m_card_add_signal;
}
sigc::signal<void(CardWidget*)>& CardlistWidget::signal_card_removed() {
    return m_card_remove_signal;
}

sigc::signal<void(CardWidget*, CardWidget*, bool)>&
CardlistWidget::signal_card_reorder() {
    return m_card_reorder_signal;
}

sigc::signal<void(CardWidget*, CardlistWidget*, CardWidget*)>&
CardlistWidget::signal_card_received() {
    return m_card_received_signal;
}

void CardlistWidget::setup_drag_and_drop() {
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->signal_prepare().connect(
        [this, drag_source_c](double x, double y) {
            Glib::Value<CardlistPayload> value_cardlist_p;
            value_cardlist_p.init(Glib::Value<CardlistPayload>::value_type());
            value_cardlist_p.set(CardlistPayload{this});
            auto cardlist_icon = Gtk::WidgetPaintable::create(*this);
            drag_source_c->set_icon(cardlist_icon, x, y);
            return Gdk::ContentProvider::create(value_cardlist_p);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag) {
            this->set_opacity(0.5);
            this->board.set_scroll();

            spdlog::get("ui")->debug("[CardlistWidget.dnd] (\"{}\"): On drag",
                                     get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            this->board.set_scroll(false);

            spdlog::get("ui")->debug(
                "[CardlistWidget.dnd] (\"{}\"): Canceled drag event",
                get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
            this->set_opacity(1);
            this->board.set_scroll(false);

            spdlog::get("ui")->debug(
                "[CardlistWidget.dnd] (\"{}\"): Stopped being draggeed",
                get_name());
        });
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    m_header.add_controller(drag_source_c);

    auto drop_target_cardlist = Gtk::DropTarget::create(
        Glib::Value<CardlistPayload>::value_type(), Gdk::DragAction::MOVE);
    drop_target_cardlist->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->board.set_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<CardlistPayload>::value_type())) {
                Glib::Value<CardlistPayload> dropped_value;
                dropped_value.init(value.gobj());

                CardlistWidget* dropped_cardlist =
                    dropped_value.get().cardlist_widget;

                if (dropped_cardlist == this) {
                    spdlog::get("app")->warn(
                        "[CardlistWidget.dnd] Card list(\"{}\") has been "
                        "dropped on itself.",
                        get_name());
                    this->remove_css_class("cardlist-to-drop");
                    return true;
                }

                this->board.reorder(*dropped_cardlist, *this);
                dropped_cardlist->set_opacity(1);

                this->remove_css_class("cardlist-to-drop");

                spdlog::get("app")->info(
                    "Card list (\"{}\") dropped on Card list (\"{}\")",
                    dropped_cardlist->get_name(), get_name());
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_cardlist);

    auto drop_target_card = Gtk::DropTarget::create(
        Glib::Value<CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_card->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->board.set_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<CardWidget*>::value_type())) {
                Glib::Value<CardWidget*> dropped_value;
                dropped_value.init(value.gobj());

                auto dropped_card = dropped_value.get();
                if (!this->is_child(*dropped_card)) {
                    receive(*dropped_card);
                    spdlog::get("app")->info(
                        "[TODO] Card (\"{}\") dropped on card list (\"{}\")",
                        dropped_card->get_title(), get_name());
                }

                this->remove_css_class("cardlist-to-drop");
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_card);
}

void CardlistWidget::cleanup() {
    m_header.unparent();
    m_popover.unparent();
    m_scr_window.unparent();
}
}  // namespace ui
