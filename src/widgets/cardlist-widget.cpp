#include "cardlist-widget.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>

#include "board-widget.h"
#include "card.h"
#include "glib.h"

extern "C" {
static void cardlist_class_init(void* klass, void* user_data) {
    g_return_if_fail(GTK_IS_WIDGET_CLASS(klass));
    gtk_widget_class_set_css_name(GTK_WIDGET_CLASS(klass), "cardlist");
}
}

ui::CardlistInit::CardlistInit() : Glib::ExtraClassInit(cardlist_class_init) {}

ui::CardlistWidget::CardlistWidget(BoardWidget& board,
                                   std::shared_ptr<CardList> cardlist_refptr,
                                   bool is_new)
    : Glib::ObjectBase{"CardlistWidget"},
      CardlistInit{},
      Gtk::Box{Gtk::Orientation::VERTICAL},
      add_card_button{_("Add card")},
      root{Gtk::Orientation::VERTICAL},
      card_widgets{},
      board{board},
      cardlist{cardlist_refptr},
      is_new{is_new},
      cardlist_header{cardlist_refptr->get_name(), "title-2",
                      "title-2"} {
    add_css_class("cardlist");
    set_halign(Gtk::Align::START);
    set_size_request(CARDLIST_MAX_WIDTH, -1);
    setup_drag_and_drop();

    if (is_new) {
        cardlist_header.to_editing_mode();
    }
    cardlist_header.add_option_button(_("Remove"), "remove", [this]() {
        this->board.remove_cardlist(*this);
    });
    cardlist_header.signal_on_confirm().connect(
        [this](const std::string& label) {
            this->cardlist->set_name(label);
            this->is_new = false;
        });
    cardlist_header.signal_on_cancel().connect(
        [this](const std::string& label) {
            if (this->is_new) {
                this->board.remove_cardlist(*this);
            }
        });
    cardlist_header.set_margin_bottom(15);

    add_card_button.set_valign(Gtk::Align::CENTER);
    add_card_button.set_hexpand(true);
    add_card_button.signal_clicked().connect(
        [this]() { this->add(Card{_("New Card")}, true); });
    root.append(add_card_button);

    append(cardlist_header);

    for (auto& card : cardlist_refptr->get_cards()) {
        _add(card);
    }

    root.set_vexpand();
    root.set_spacing(15);

    Gtk::ScrolledWindow scr_window{};
    scr_window.set_child(root);
    scr_window.set_size_request(CARDLIST_MAX_WIDTH, -1);
    scr_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    append(scr_window);

    auto shortcut_controller = Gtk::ShortcutController::create();
    shortcut_controller->set_scope(Gtk::ShortcutScope::LOCAL);
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>N"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                auto n_cardlist =
                    this->board.add_cardlist(CardList{_("New Cardlist")}, true);

                this->board.reorder_cardlist(*n_cardlist, *this);
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Delete"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                this->board.remove_cardlist(*this);
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Left"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                CardlistWidget* previous_cardlist =
                    static_cast<CardlistWidget*>(this->get_prev_sibling());

                if (previous_cardlist) {
                    this->board.reorder_cardlist(*previous_cardlist, *this);
                }
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Right"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                Widget* maybe_cardlist = this->get_next_sibling();

                if (!G_TYPE_CHECK_INSTANCE_TYPE(maybe_cardlist->gobj(),
                                                Gtk::Button::get_type())) {
                    CardlistWidget* cardlist_widget =
                        static_cast<CardlistWidget*>(maybe_cardlist);
                    this->board.reorder_cardlist(*this, *cardlist_widget);
                }
                return true;
            })));
    add_controller(shortcut_controller);
}

ui::CardlistWidget::~CardlistWidget() {}

void ui::CardlistWidget::reorder(ui::CardWidget& next,
                                 ui::CardWidget& sibling) {
    root.reorder_child_after(next, sibling);
    cardlist->reorder(*next.get_card(), *sibling.get_card());

    spdlog::get("ui")->debug(
        "CardWidget \"{}\" has been reordered after CardWidget \"{}\"",
        next.get_card()->get_name(), sibling.get_card()->get_name());
}

const std::vector<ui::CardWidget*>& ui::CardlistWidget::get_card_widgets() {
    return card_widgets;
}

void ui::CardlistWidget::setup_drag_and_drop() {
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->signal_prepare().connect(
        [this, drag_source_c](double x, double y) {
            Glib::Value<ui::CardlistWidget*> value_cardlist_p;
            value_cardlist_p.init(
                Glib::Value<ui::CardlistWidget*>::value_type());
            value_cardlist_p.set(this);
            auto cardlist_icon = Gtk::WidgetPaintable::create(*this);
            drag_source_c->set_icon(cardlist_icon, x, y);
            return Gdk::ContentProvider::create(value_cardlist_p);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag) {
            this->set_opacity(0.5);
            this->board.set_on_scroll();

            spdlog::get("ui")->debug(
                "CardlistWidget \"{}\" has started dragging",
                this->get_cardlist()->get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            this->board.set_on_scroll(false);

            spdlog::get("ui")->debug(
                "CardlistWidget \"{}\" has cancelled dragging",
                this->get_cardlist()->get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
            this->set_opacity(1);
            this->board.set_on_scroll(false);

            spdlog::get("ui")->debug("CardlistWidget \"{}\" has ended dragging",
                                     this->get_cardlist()->get_name());
        });
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    cardlist_header.add_controller(drag_source_c);

    auto drop_target_cardlist = Gtk::DropTarget::create(
        Glib::Value<ui::CardlistWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_cardlist->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->board.set_on_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardlistWidget*>::value_type())) {
                Glib::Value<ui::CardlistWidget*> dropped_value;
                dropped_value.init(value.gobj());

                ui::CardlistWidget* dropped_cardlist = dropped_value.get();

                if (dropped_cardlist == this) {
                    spdlog::get("ui")->warn(
                        "CardlistWidget \"{}\" has been dropped on itself. "
                        "Nothing happens",
                        this->get_cardlist()->get_name());
                    return true;
                }

                this->board.reorder_cardlist(*dropped_cardlist, *this);
                dropped_cardlist->set_opacity(1);

                spdlog::get("ui")->debug(
                    "CardlistWidget \"{}\" has been dropped on CardlistWidget "
                    "\"{}\"",
                    dropped_cardlist->get_cardlist()->get_name(),
                    this->get_cardlist()->get_name());
                return true;
            }
            return false;
        },
        false);
    cardlist_header.add_controller(drop_target_cardlist);

    auto drop_target_card = Gtk::DropTarget::create(
        Glib::Value<ui::CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_card->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->board.set_on_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardWidget*>::value_type())) {
                Glib::Value<ui::CardWidget*> dropped_value;
                dropped_value.init(value.gobj());

                auto dropped_card = dropped_value.get();
                if (!this->is_child(dropped_card)) {
                    auto card_in_dropped = dropped_card->get_card();
                    dropped_card->remove_from_parent();
                    this->add(*card_in_dropped);

                    spdlog::get("ui")->debug(
                        "CardWidget \"{}\" has been dropped on CardlistWidget "
                        "\"{}\"",
                        dropped_card->get_card()->get_name(),
                        this->get_cardlist()->get_name());
                }
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_card);
}

void ui::CardlistWidget::remove(ui::CardWidget* card) {
    spdlog::get("ui")->debug(
        "CardWidget \"{}\" has been removed from CardlistWidget \"{}\"",
        card->get_card()->get_name(), cardlist->get_name());

    root.remove(*card);
    cardlist->remove(*card->get_card());
    std::erase(card_widgets, card);
}

ui::CardWidget* ui::CardlistWidget::add(const Card& card, bool editing_mode) {
    return _add(cardlist->add(card), editing_mode);
}

const std::shared_ptr<CardList>& ui::CardlistWidget::get_cardlist() {
    return cardlist;
}

bool ui::CardlistWidget::is_child(ui::CardWidget* card) {
    for (auto& card_ : card_widgets) {
        if (card == card_) return true;
    }
    return false;
}

ui::CardWidget* ui::CardlistWidget::_add(const std::shared_ptr<Card>& card,
                                         bool editing_mode) {
    auto cardwidget = Gtk::make_managed<CardWidget>(card, editing_mode);
    card_widgets.push_back(cardwidget);
    cardwidget->set_cardlist(this);
    root.append(*cardwidget);
    root.reorder_child_after(add_card_button, *cardwidget);

    spdlog::get("ui")->debug(
        "CardWidget \"{}\" has been added to CardlistWidget \"{}\"",
        card->get_name(), cardlist->get_name());

    return cardwidget;
}
