#include "cardlist-widget.h"

#include <glibmm/i18n.h>

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
      cardlist{cardlist_refptr},
      is_new{is_new},
      cardlist_header{cardlist_refptr->get_name(), "cardlist-title",
                      "cardlist-title"} {
    add_css_class("rich-list");
    set_valign(Gtk::Align::START);
    set_vexpand(true);
    set_halign(Gtk::Align::START);
    set_size_request(CARDLIST_MAX_WIDTH, CARDLIST_MAX_WIDTH * 2);
    set_selection_mode(Gtk::SelectionMode::NONE);
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

    add_card_button.set_valign(Gtk::Align::CENTER);
    add_card_button.set_halign(Gtk::Align::FILL);
    add_card_button.set_hexpand(true);
    add_card_button.signal_clicked().connect(
        [this]() { this->add_card(Card{_("New Card")}, true); });
    root.append(add_card_button);

    append(cardlist_header);
    for (auto& card : cardlist_refptr->get_cards()) {
        auto cur_builder = Gtk::Builder::create_from_resource(
            "/io/github/smolblackcat/Progress/card-widget.ui");
        auto cardwidget =
            Gtk::manage(cur_builder->get_widget_derived<CardWidget>(
                cur_builder, "card-root", card));
        cards_tracker.push_back(cardwidget);
        cardwidget->set_cardlist(this);
        root.append(*cardwidget);
        root.reorder_child_after(add_card_button, *cardwidget);
    }

    root.set_size_request(CARDLIST_MAX_WIDTH, CARDLIST_MAX_WIDTH);
    root.set_valign(Gtk::Align::FILL);
    root.set_vexpand();
    root.set_spacing(15);
    root.set_margin_top(4);

    Gtk::ScrolledWindow scr_window{};
    scr_window.set_child(root);
    scr_window.set_size_request(CARDLIST_MAX_WIDTH, CARDLIST_MAX_WIDTH * 2);
    scr_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scr_window.get_vscrollbar()->set_visible(false);
    append(scr_window);

    // Makes the header and the list itself non-selectable
    get_row_at_index(0)->set_activatable(false);
    get_row_at_index(1)->set_activatable(false);
}

ui::CardlistWidget::~CardlistWidget() {}

void ui::CardlistWidget::reorder_cardwidget(ui::CardWidget& next,
                                            ui::CardWidget& sibling) {
    root.reorder_child_after(next, sibling);
    cardlist->reorder_card(*next.get_card(), *sibling.get_card());
}

const std::vector<ui::CardWidget*>&
ui::CardlistWidget::get_cardwidget_vector() {
    return cards_tracker;
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
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            this->board.set_on_scroll(false);
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
            this->set_opacity(1);
            this->board.set_on_scroll(false);
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
                    return true;
                }

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
            this->board.set_on_scroll(false);
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
    cardlist->remove_card(*card->get_card());

    for (size_t i = 0; i < cards_tracker.size(); i++) {
        if (cards_tracker[i] == card) {
            cards_tracker.erase(cards_tracker.begin() + i);
        }
    }
}

ui::CardWidget* ui::CardlistWidget::add_card(const Card& card,
                                             bool editing_mode) {
    auto card_builder = Gtk::Builder::create_from_resource(
        "/io/github/smolblackcat/Progress/card-widget.ui");
    auto new_card = Gtk::manage(Gtk::Builder::get_widget_derived<CardWidget>(
        card_builder, "card-root", cardlist->add_card(card), editing_mode));
    new_card->set_cardlist(this);

    cards_tracker.push_back(new_card);
    root.append(*new_card);
    root.reorder_child_after(add_card_button, *new_card);
    return new_card;
}

const std::shared_ptr<CardList>& ui::CardlistWidget::get_cardlist() {
    return cardlist;
}

bool ui::CardlistWidget::is_child(ui::CardWidget* card) {
    for (auto& card_ : cards_tracker) {
        if (card == card_) return true;
    }
    return false;
}

// ui::EditableLabelHeader& ui::CardlistWidget::get_header() {
//     return cardlist_header;
// }
