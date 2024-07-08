#include "cardlist-widget.h"

#include <glibmm/i18n.h>

#include <format>

#include "board-widget.h"
#include "card.h"

ui::CardlistWidget::CardlistWidget(BoardWidget& board,
                                   std::shared_ptr<CardList> cardlist_refptr,
                                   bool is_new)
    : Gtk::ListBox{},
      add_card_button{_("Add card")},
      root{Gtk::Orientation::VERTICAL},
      cards_tracker{},
      board{board},
      cardlist_refptr{cardlist_refptr},
      is_new{is_new},
      cardlist_header{cardlist_refptr->get_name(), "cardlist-title",
                      "cardlist-title"} {
    add_css_class("rich-list");
    set_valign(Gtk::Align::START);
    set_vexpand(true);
    set_halign(Gtk::Align::START);
    set_size_request(CARDLIST_SIZE, CARDLIST_SIZE * 2);
    set_selection_mode(Gtk::SelectionMode::NONE);
    setup_drag_and_drop();

    if (is_new) {
        cardlist_header.to_editing_mode();
    }
    cardlist_header.add_option_button(_("Remove"), "remove", [this]() {
        this->board.remove_cardlist(*this);
    });
    cardlist_header.signal_confirm().connect([this](std::string label) {
        this->cardlist_refptr->set_name(label);
        this->is_new = false;
    });
    cardlist_header.signal_cancel().connect([this](std::string label) {
        if (this->is_new) {
            this->board.remove_cardlist(*this);
        }
    });

    add_card_button.set_valign(Gtk::Align::CENTER);
    add_card_button.set_halign(Gtk::Align::START);
    add_card_button.set_hexpand(false);
    add_card_button.signal_clicked().connect(
        [this]() { this->add_card(Card{_("New Card")}, true); });
    root.append(add_card_button);

    append(cardlist_header);
    for (auto& card : cardlist_refptr->get_card_vector()) {
        auto cardwidget = Gtk::make_managed<CardWidget>(card);
        cards_tracker.push_back(cardwidget);
        cardwidget->set_cardlist(this);
        root.append(*cardwidget);
        root.reorder_child_after(add_card_button, *cardwidget);
    }

    root.set_size_request(CARDLIST_SIZE, CARDLIST_SIZE);
    root.set_valign(Gtk::Align::FILL);
    root.set_vexpand();
    root.set_spacing(15);
    root.set_margin_top(4);

    Gtk::ScrolledWindow scr_window{};
    scr_window.set_child(root);
    scr_window.set_size_request(CARDLIST_SIZE, CARDLIST_SIZE * 2);
    scr_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    append(scr_window);

    // Makes the header and the list itself non-selectable
    get_row_at_index(0)->set_activatable(false);
    get_row_at_index(1)->set_activatable(false);
}

void ui::CardlistWidget::reorder_cardwidget(ui::CardWidget& next,
                                            ui::CardWidget& sibling) {
    root.reorder_child_after(next, sibling);
    cardlist_refptr->reorder_card(next.get_card(), sibling.get_card());
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
            this->board.on_drag = true;
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            this->board.on_drag = false;
            return true;
        },
        false);
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    cardlist_header.add_controller(drag_source_c);

    auto drop_target_cardlist = Gtk::DropTarget::create(
        Glib::Value<ui::CardlistWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_cardlist->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->board.on_drag = false;
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardlistWidget*>::value_type())) {
                Glib::Value<ui::CardlistWidget*> dropped_value;
                dropped_value.init(value.gobj());

                ui::CardlistWidget* dropped_cardlist = dropped_value.get();
                this->board.reorder_cardlist(*dropped_cardlist, *this);
                dropped_cardlist->set_opacity(1);
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
            this->board.on_drag = false;
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardWidget*>::value_type())) {
                Glib::Value<ui::CardWidget*> dropped_value;
                dropped_value.init(value.gobj());

                auto dropped_card = dropped_value.get();
                if (!this->is_child(dropped_card)) {
                    auto card_in_dropped = dropped_card->get_card();
                    dropped_card->remove_from_parent();
                    this->add_card(*card_in_dropped);
                }
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_card);
}

void ui::CardlistWidget::remove_card(ui::CardWidget* card) {
    root.remove(*card);
    cardlist_refptr->remove_card(*card->get_card());

    for (size_t i = 0; i < cards_tracker.size(); i++) {
        if (cards_tracker[i] == card) {
            cards_tracker.erase(cards_tracker.begin() + i);
        }
    }
}

ui::CardWidget* ui::CardlistWidget::add_card(const Card& card,
                                             bool editing_mode) {
    auto new_card = Gtk::make_managed<ui::CardWidget>(
        cardlist_refptr->add_card(card), editing_mode);
    new_card->set_cardlist(this);

    if (editing_mode) {
        new_card->to_editing_mode();
    }

    cards_tracker.push_back(new_card);
    root.append(*new_card);
    root.reorder_child_after(add_card_button, *new_card);
    return new_card;
}

std::shared_ptr<CardList>& ui::CardlistWidget::get_cardlist_refptr() {
    return cardlist_refptr;
}

bool ui::CardlistWidget::is_child(ui::CardWidget* card) {
    for (auto& card_ : cards_tracker) {
        if (card == card_) return true;
    }
    return false;
}

ui::EditableLabelHeader& ui::CardlistWidget::get_header() {
    return cardlist_header;
}
