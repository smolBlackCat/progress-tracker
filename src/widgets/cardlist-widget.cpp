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

CardlistWidget::CardlistWidget(BoardWidget& board,
                               const std::shared_ptr<CardList>& cardlist,
                               bool is_new)
    : Glib::ObjectBase{"CardlistWidget"},
      CardlistInit{},
      BaseItem{Gtk::Orientation::VERTICAL, 0},
      m_add_card_button{_("Add card")},
      m_root{Gtk::Orientation::VERTICAL},
      m_cards{},
      board{board},
      m_cardlist{cardlist},
      m_new{is_new},
      m_header{cardlist->get_name(), "title-2", "title-2"},
      m_scr_window{} {
    set_layout_manager(Gtk::BoxLayout::create(Gtk::Orientation::VERTICAL));
    add_css_class("cardlist");
    set_halign(Gtk::Align::START);
    set_size_request(CARDLIST_MAX_WIDTH, -1);
    setup_drag_and_drop();

    if (is_new) {
        m_header.to_editing_mode();
    }
    m_header.add_option_button(_("Remove"), "remove", [this]() {
        this->board.remove_cardlist(*this);
    });
    m_header.signal_on_confirm().connect([this](std::string label) {
        this->m_cardlist->set_name(label);
        this->m_new = false;
    });
    m_header.signal_on_cancel().connect([this](std::string label) {
        if (this->m_new) {
            this->board.remove_cardlist(*this);
        }
    });

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
    m_add_card_button.signal_clicked().connect(
        [this]() { this->add(Card{_("New Card")}, true); });
    m_root.append(m_add_card_button);

    m_header.insert_at_start(*this);

    // for (const auto& card : m_cardlist->container()) {
    //     __add(card);
    // }

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
              this->board.insert_new_cardlist_after(CardList{_("New Cardlist")},
                                                    this);
              return true;
          }},
         {"<Control>Delete",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              this->board.remove_cardlist(*this);
              return true;
          }},
         {"<Control>Left",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              CardlistWidget* previous_cardlist =
                  static_cast<CardlistWidget*>(this->get_prev_sibling());
              if (previous_cardlist) {
                  this->board.reorder_cardlist(*previous_cardlist, *this);
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
                  this->board.reorder_cardlist(*this, *cardlist_widget);
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

    auto drop_motion_controller = Gtk::DropControllerMotion::create();
    // drop_motion_controller->signal_motion().connect([this](double x, double
    // y) {
    //     this->add_css_class("cardlist-to-drop");
    // });
    // drop_motion_controller->signal_leave().connect(
    //     [this]() { this->remove_css_class("cardlist-to-drop"); });
    add_controller(drop_motion_controller);

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

void CardlistWidget::reorder(CardWidget& next, CardWidget& sibling) {
    ReorderingType reordering =
        m_cardlist->container().reorder(*next.get_card(), *sibling.get_card());

    switch (reordering) {
        case ReorderingType::DOWNUP: {
            auto sibling_sibling = sibling.get_prev_sibling();
            if (!sibling_sibling) {
                m_root.reorder_child_at_start(next);
            } else {
                m_root.reorder_child_after(next, *sibling_sibling);
            }
            spdlog::get("ui")->debug(
                "[CardlistWidget] CardWidget \"{}\" was inserted before "
                "CardWidget \"{}\"",
                next.get_card()->get_name(), sibling.get_card()->get_name());
            break;
        }
        case ReorderingType::UPDOWN: {
            m_root.reorder_child_after(next, sibling);

            spdlog::get("ui")->debug(
                "[CardlistWidget] CardWidget \"{}\" was inserted after "
                "CardWidget \"{}\"",
                next.get_card()->get_name(), sibling.get_card()->get_name());
            break;
        }
        case ReorderingType::INVALID: {
            spdlog::get("ui")->warn("[CardlistWidget] Invalid reorder request");
            break;
        }
    }
}

void CardlistWidget::remove(CardWidget& card) {
    spdlog::get("ui")->debug(
        "[CardlistWidget] CardWidget \"{}\" has been removed from "
        "CardlistWidget \"{}\"",
        card.get_card()->get_name(), m_cardlist->get_name());

    m_root.remove(card);
    m_cardlist->container().remove(*card.get_card());
    std::erase(m_cards, &card);

    remove_card_signal.emit(&card);
}

CardWidget* CardlistWidget::add(const Card& card, bool editing_mode) {
    return __add(m_cardlist->container().append(card), editing_mode);
}

CardWidget* CardlistWidget::insert_new_card_after(const Card& card,
                                                  ui::CardWidget* sibling) {
    auto cardwidget = Gtk::make_managed<CardWidget>(
        m_cardlist->container().insert_after(card, *sibling->get_card()), true);
    m_cards.push_back(cardwidget);
    cardwidget->set_cardlist(this);
    m_root.insert_child_after(*cardwidget, *sibling);

    spdlog::get("ui")->debug(
        "[CardlistWidget] CardWidget \"{}\" has been added to CardlistWidget "
        "\"{}\"",
        card.get_name(), m_cardlist->get_name());

    add_card_signal.emit(cardwidget);

    return cardwidget;
}

bool CardlistWidget::is_child(CardWidget& card) {
    for (auto& card_ : m_cards) {
        if (&card == card_) return true;
    }
    return false;
}

const std::vector<CardWidget*>& CardlistWidget::cards() { return m_cards; }

const std::shared_ptr<CardList>& CardlistWidget::cardlist() {
    return m_cardlist;
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

            spdlog::get("ui")->debug(
                "[CardlistWidget] CardlistWidget \"{}\" is being dragged",
                this->cardlist()->get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            this->board.set_scroll(false);

            spdlog::get("ui")->debug(
                "[CardlistWidget] CardlistWidget \"{}\" dragging has been "
                "canceled",
                this->cardlist()->get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
            this->set_opacity(1);
            this->board.set_scroll(false);

            spdlog::get("ui")->debug(
                "[CardlistWidget] CardlistWidget \"{}\" stopped being draggeed",
                this->cardlist()->get_name());
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
                    spdlog::get("ui")->warn(
                        "[CardlistWidget] CardlistWidget \"{}\" has been "
                        "dropped on itself.",
                        this->cardlist()->get_name());
                    this->remove_css_class("cardlist-to-drop");
                    return true;
                }

                this->board.reorder_cardlist(*dropped_cardlist, *this);
                dropped_cardlist->set_opacity(1);

                this->remove_css_class("cardlist-to-drop");

                spdlog::get("ui")->debug(
                    "[CardlistWidget] CardlistWidget \"{}\" has been dropped "
                    "on CardlistWidget \"{}\"",
                    dropped_cardlist->cardlist()->get_name(),
                    this->cardlist()->get_name());
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
                    auto card_in_dropped = dropped_card->get_card();
                    dropped_card->remove_from_parent();
                    this->add(*card_in_dropped);

                    spdlog::get("ui")->debug(
                        "[CardlistWidget] CardWidget \"{}\" has been dropped "
                        "on CardlistWidget \"{}\"",
                        dropped_card->get_card()->get_name(),
                        this->cardlist()->get_name());
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

CardWidget* CardlistWidget::__add(const std::shared_ptr<Card>& card,
                                  bool editing_mode) {
    auto cardwidget = Gtk::make_managed<CardWidget>(card, editing_mode);
    m_cards.push_back(cardwidget);
    cardwidget->set_cardlist(this);
    m_root.append(*cardwidget);
    m_root.reorder_child_after(m_add_card_button, *cardwidget);

    spdlog::get("ui")->debug(
        "[CardlistWidget] CardWidget \"{}\" has been added to CardlistWidget "
        "\"{}\"",
        card->get_name(), m_cardlist->get_name());

    add_card_signal.emit(cardwidget);

    return cardwidget;
}

sigc::signal<void(CardWidget*)>& CardlistWidget::signal_card_added() {
    return add_card_signal;
}
sigc::signal<void(CardWidget*)>& CardlistWidget::signal_card_removed() {
    return remove_card_signal;
}
}  // namespace ui

