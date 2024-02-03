#include "board-widget.h"

#include <format>
#include <iostream>

/**
 * TODO: High memory is allocated in setting background, mainly when the
 * background image is high. Should I try to compress it?
 */
ui::BoardWidget::BoardWidget()
    : Gtk::ScrolledWindow{},
      root{Gtk::Orientation::HORIZONTAL},
      add_button{"Add List"},
      cardlist_vector{},
      board{nullptr},
      board_card_button{nullptr} {
    set_child(root);
    set_name("board-root");

    root.set_spacing(25);
    root.set_margin(10);

    css_provider_refptr = Gtk::CssProvider::create();
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider_refptr, GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_provider_refptr->load_from_data(CSS_FORMAT);

    add_button.signal_clicked().connect([this]() {
        add_cardlist(board->add_cardlist(CardList{"New CardList"}));
    });
    add_button.set_halign(Gtk::Align::START);
    add_button.set_valign(Gtk::Align::START);
    add_button.set_hexpand();

    root.append(add_button);
}

ui::BoardWidget::~BoardWidget() {}

void ui::BoardWidget::set(Board* board, BoardCardButton* board_card_button) {
    if (!(board || board_card_button)) return;

    clear();
    this->board = board;
    this->board_card_button = board_card_button;

    // Code updating cardlists
    for (auto& cardlist : board->get_cardlists()) {
        add_cardlist(cardlist);
    }
    set_background();
}

void ui::BoardWidget::clear() {
    if (cardlist_vector.empty()) return;

    for (auto cardlist_widget : cardlist_vector) {
        root.remove(*cardlist_widget);
    }
    cardlist_vector.clear();
}

bool ui::BoardWidget::save() {
    bool success;
    if (board->is_modified()) {
        success = board->save_as_xml();
    }
    board_card_button->update(board);
    delete board;
    board = nullptr;
    return success;
}

void ui::BoardWidget::add_cardlist(std::shared_ptr<CardList> cardlist_refptr) {
    auto new_cardlist =
        Gtk::make_managed<ui::CardlistWidget>(*this, cardlist_refptr);
    cardlist_vector.push_back(new_cardlist);

    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->signal_prepare().connect(
        [new_cardlist, drag_source_c](double x, double y) {
            Glib::Value<ui::CardlistWidget*> value_cardlistptr;
            value_cardlistptr.init(
                Glib::Value<ui::CardlistWidget*>::value_type());
            value_cardlistptr.set(new_cardlist);
            auto cardlist_widget_paintable =
                Gtk::WidgetPaintable::create(*new_cardlist);
            drag_source_c->set_icon(cardlist_widget_paintable, x, y);
            return Gdk::ContentProvider::create(value_cardlistptr);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [new_cardlist](const Glib::RefPtr<Gdk::Drag>& drag) {
            new_cardlist->set_opacity(0.5);
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [new_cardlist](const Glib::RefPtr<Gdk::Drag>& drag,
                       Gdk::DragCancelReason reason) {
            new_cardlist->set_opacity(1);
            return true;
        },
        false);
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    new_cardlist->get_header().add_controller(drag_source_c);

    auto drop_target_cardlist = Gtk::DropTarget::create(
        Glib::Value<ui::CardlistWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_cardlist->signal_drop().connect(
        [this, new_cardlist](const Glib::ValueBase& value, double x, double y) {
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardlistWidget*>::value_type())) {
                Glib::Value<ui::CardlistWidget*> dropped_value;
                dropped_value.init(value.gobj());

                ui::CardlistWidget* dropped_cardlist = dropped_value.get();
                root.reorder_child_after(*dropped_cardlist, *new_cardlist);
                board->reorder_cardlist(dropped_cardlist->get_cardlist_refptr(),
                                        new_cardlist->get_cardlist_refptr());
                dropped_cardlist->set_opacity(1);
                return true;
            }
            return false;
        },
        false);
    new_cardlist->get_header().add_controller(drop_target_cardlist);

    auto drop_target_card = Gtk::DropTarget::create(
        Glib::Value<ui::CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_card->signal_drop().connect(
        [new_cardlist](const Glib::ValueBase& value, double x, double y) {
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardWidget*>::value_type())) {
                Glib::Value<ui::CardWidget*> dropped_value;
                dropped_value.init(value.gobj());

                auto dropped_card = dropped_value.get();
                if (!new_cardlist->is_child(dropped_card)) {
                    auto card_in_dropped = dropped_card->get_card();
                    dropped_card->remove();
                    new_cardlist->add_card(card_in_dropped);
                    new_cardlist->get_cardlist_refptr()->add_card(
                        *card_in_dropped);
                }
                return true;
            }
            return false;
        },
        false);
    new_cardlist->add_controller(drop_target_card);

    // The new cardlist should be appended before the add_button
    root.append(*new_cardlist);
    root.reorder_child_after(add_button, *new_cardlist);
}

bool ui::BoardWidget::set_background() {
    if (!board) return false;

    if (board->get_background_type() == "colour") {
        css_provider_refptr->load_from_data(
            std::format(CSS_FORMAT_RGB, board->get_background()));
        std::cout << "Colour background set" << std::endl;
        return true;
    } else if (board->get_background_type() == "file") {
        css_provider_refptr->load_from_data(
            std::format(CSS_FORMAT_FILE, board->get_background()));
        std::cout << "File background set" << std::endl;
        return true;
    }

    return false;
}

bool ui::BoardWidget::remove_cardlist(ui::CardlistWidget& cardlist) {
    root.remove(cardlist);
    std::erase(cardlist_vector, &cardlist);
    board->remove_cardlist(*(cardlist.get_cardlist_refptr()));
    return true;
}