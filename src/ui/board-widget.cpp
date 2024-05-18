#include "board-widget.h"

#include <glibmm/i18n.h>

#include <chrono>
#include <format>
#include <iostream>

#include "cardlist-widget.h"
#include "window.h"

using namespace std::chrono_literals;

/**
 * TODO: High memory is allocated in setting background, mainly when the
 * background image is high. Should I try to compress it?
 */
ui::BoardWidget::BoardWidget(ui::ProgressWindow& app_window)
    : Gtk::ScrolledWindow{},
      root{Gtk::Orientation::HORIZONTAL},
      add_button{_("Add List")},
      app_window{app_window},
      on_drag{false} {
    set_child(root);
    set_name("board-root");

    setup_auto_scrolling();

    root.set_spacing(25);
    root.set_margin(10);

    css_provider_refptr = Gtk::CssProvider::create();
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider_refptr, GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_provider_refptr->load_from_data(CSS_FORMAT);

    add_button.signal_clicked().connect([this]() {
        add_cardlist(board->add_cardlist(CardList{_("New CardList")}), true);
    });
    add_button.set_halign(Gtk::Align::START);
    add_button.set_valign(Gtk::Align::START);
    add_button.set_hexpand();

    root.append(add_button);
}

ui::BoardWidget::~BoardWidget() {}

void ui::BoardWidget::set(Board* board, BoardCardButton* board_card_button) {
    if (board && board_card_button) {
        clear();
        this->board = board;
        this->board_card_button = board_card_button;

        for (auto& cardlist : board->get_cardlists()) {
            add_cardlist(cardlist);
        }
        set_background(board->get_background());
    }
}

void ui::BoardWidget::clear() {
    if (!cardlist_vector.empty()) {
        for (auto& cardlist_widget : cardlist_vector) {
            root.remove(*cardlist_widget);
        }
        cardlist_vector.clear();
    }
}

bool ui::BoardWidget::save(bool free) {
    bool success;
    if (board->is_modified()) {
        success = board->save_as_xml();
    }
    board_card_button->update(board);
    if (free) {
        delete board;
        board = nullptr;
    }
    return success;
}

void ui::BoardWidget::add_cardlist(std::shared_ptr<CardList> cardlist_refptr,
                                   bool is_new) {
    auto new_cardlist =
        Gtk::make_managed<ui::CardlistWidget>(*this, cardlist_refptr, is_new);
    cardlist_vector.push_back(new_cardlist);

    setup_drag_and_drop(new_cardlist);

    root.append(*new_cardlist);
    root.reorder_child_after(add_button, *new_cardlist);
}

bool ui::BoardWidget::remove_cardlist(ui::CardlistWidget& cardlist) {
    root.remove(cardlist);
    std::erase(cardlist_vector, &cardlist);
    board->remove_cardlist(*cardlist.get_cardlist_refptr());
    return true;
}

void ui::BoardWidget::set_background(const std::string& background) {
    BackgroundType bg_type = board->set_background(background);
    switch (bg_type) {
        case BackgroundType::COLOR: {
            css_provider_refptr->load_from_data(
                std::format(CSS_FORMAT_RGB, background));
            break;
        }
        case BackgroundType::IMAGE: {
            css_provider_refptr->load_from_data(
                std::format(CSS_FORMAT_FILE, background));
            break;
        }
        case BackgroundType::INVALID: {
            css_provider_refptr->load_from_data(
                std::format(CSS_FORMAT_RGB, Board::BACKGROUND_DEFAULT));
            break;
        }
    }
}

std::string ui::BoardWidget::get_background() {
    return board ? board->get_background() : "";
}

void ui::BoardWidget::set_board_name(const std::string& board_name) {
    if (board) {
        app_window.set_title(board_name);
        board->set_name(board_name);
    }
}

std::string ui::BoardWidget::get_board_name() {
    return board ? board->get_name() : "";
}

void ui::BoardWidget::set_filepath(const std::string& board_filepath) {
    if (board) {
        board->set_filepath(board_filepath);
    }
}

std::string ui::BoardWidget::get_filepath() {
    return board ? board->get_filepath() : "";
}

void ui::BoardWidget::setup_auto_scrolling() {
    auto drop_controller_motion_c = Gtk::DropControllerMotion::create();
    drop_controller_motion_c->signal_motion().connect(
        [this](double x, double y) {
            this->x = x;
            this->y = y;
        });
    this->add_controller(drop_controller_motion_c);

    Glib::signal_timeout().connect(
        [this]() {
            double cur_max_width = this->get_width();
            auto hadjustment = this->get_hadjustment();
            double lower = hadjustment->get_lower();
            double upper = hadjustment->get_upper();

            if (on_drag) {
                if (x >= (cur_max_width * 0.8)) {
                    double new_value = hadjustment->get_value() + 3;
                    hadjustment->set_value(new_value >= upper ? upper
                                                              : new_value);
                } else if (x <= (cur_max_width * 0.2)) {
                    double new_value = hadjustment->get_value() - 3;
                    hadjustment->set_value(new_value <= lower ? lower
                                                              : new_value);
                }
            }
            return true;
        },
        10);
}

void ui::BoardWidget::setup_drag_and_drop(ui::CardlistWidget* new_cardlist) {
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
        [this, new_cardlist](const Glib::RefPtr<Gdk::Drag>& drag) {
            new_cardlist->set_opacity(0.5);
            on_drag = true;
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this, new_cardlist](const Glib::RefPtr<Gdk::Drag>& drag,
                             Gdk::DragCancelReason reason) {
            new_cardlist->set_opacity(1);
            on_drag = false;
            return true;
        },
        false);
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    new_cardlist->get_header().add_controller(drag_source_c);

    auto drop_target_cardlist = Gtk::DropTarget::create(
        Glib::Value<ui::CardlistWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_cardlist->signal_drop().connect(
        [this, new_cardlist](const Glib::ValueBase& value, double x, double y) {
            on_drag = false;
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
        [this, new_cardlist](const Glib::ValueBase& value, double x, double y) {
            on_drag = false;
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
}