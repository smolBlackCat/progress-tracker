#include "board-widget.h"

#include <glibmm/i18n.h>
#include <window.h>

#include <format>

#include "cardlist-widget.h"

/**
 * TODO: High memory is allocated in setting background, mainly when the
 * background image is high. Should I try to compress it?
 */
ui::BoardWidget::BoardWidget()
    : Gtk::ScrolledWindow{},
      root{Gtk::Orientation::HORIZONTAL},
#ifdef WIN32
      picture{},
      scr{},
      overlay{},
#endif
      add_button{_("Add List")} {

#ifdef WIN32
    set_child(overlay);
    picture.set_keep_aspect_ratio(false);
    overlay.set_child(picture);
    scr.set_child(root);
    overlay.add_overlay(scr);
    overlay.set_expand(true);
#else
    set_child(root);
#endif
    Gtk::Widget::set_name("board-root");

    setup_auto_scrolling();

    root.set_spacing(25);
    root.set_margin(10);

    css_provider_refptr = Gtk::CssProvider::create();
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider_refptr, GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_provider_refptr->load_from_data(CSS_FORMAT);

    add_button.signal_clicked().connect(
        [this]() { add_cardlist(CardList{_("New CardList")}, true); });
    add_button.set_valign(Gtk::Align::START);
    add_button.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH);
    add_button.add_css_class("add-cardlist-button");

    root.append(add_button);

    // Auto-saves the Board after 10 secs
    Glib::signal_timeout().connect(
        [this]() {
            if (this->board) {
                this->save(false);
            }
            return true;
        },
        BoardWidget::SAVE_INTERVAL);

    // Update due date labels of every card in the board every minute
    Glib::signal_timeout().connect(
        [this]() {
            if (this->board) {
                for (auto& cardlist : this->cardlist_vector) {
                    for (auto& card : cardlist->get_cardwidget_vector()) {
                        card->update_due_date_label_style();
                    }
                }
            }
            return true;
        },
        BoardWidget::SAVE_INTERVAL * 6);
}

ui::BoardWidget::~BoardWidget() {}

void ui::BoardWidget::set(std::shared_ptr<Board>& board,
                          BoardCardButton* board_card_button) {
    if (board && board_card_button) {
        this->board = board;
        this->board_card_button = board_card_button;

        for (auto& cardlist : board->get_cardlist_vector()) {
            auto new_cardlist =
                Gtk::make_managed<ui::CardlistWidget>(*this, cardlist);
            cardlist_vector.push_back(new_cardlist);

            root.append(*new_cardlist);
            root.reorder_child_after(add_button, *new_cardlist);
        }
        set_background(board->get_background(), false);
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
    if (board->get_modified()) {
        success = board->save();
        board_card_button->update(board->backend);
    }
    if (free) {
        clear();
    }
    return success;
}

ui::CardlistWidget* ui::BoardWidget::add_cardlist(const CardList& cardlist,
                                                  bool editing_mode) {
    auto new_cardlist = Gtk::make_managed<ui::CardlistWidget>(
        *this, board->add_cardlist(cardlist), editing_mode);
    cardlist_vector.push_back(new_cardlist);

    root.append(*new_cardlist);
    root.reorder_child_after(add_button, *new_cardlist);

    return new_cardlist;
}

bool ui::BoardWidget::remove_cardlist(ui::CardlistWidget& cardlist) {
    root.remove(cardlist);
    std::erase(cardlist_vector, &cardlist);
    board->remove_cardlist(*cardlist.get_cardlist());
    return true;
}

void ui::BoardWidget::reorder_cardlist(CardlistWidget& next,
                                       CardlistWidget& sibling) {
    board->reorder_cardlist(*next.get_cardlist(), *sibling.get_cardlist());
    root.reorder_child_after(next, sibling);
}

void ui::BoardWidget::set_background(const std::string& background,
                                     bool modify) {
    // Reseting background is the approach to ensure that a background is set
    // even when background turns invalid for whatever reason
    BackgroundType bg_type = board->set_background(background, modify);
    switch (bg_type) {
        case BackgroundType::COLOR: {
            css_provider_refptr->load_from_data(
                std::format(CSS_FORMAT_RGB, background));
#ifdef WIN32
            picture.set_visible(false);
#endif
            break;
        }
        case BackgroundType::IMAGE: {
#ifdef WIN32
            picture.set_filename(background);
            picture.set_visible(true);
            break;
#else
            css_provider_refptr->load_from_data(
                std::format(CSS_FORMAT_FILE, background));
            break;
#endif
        }
        case BackgroundType::INVALID: {
            css_provider_refptr->load_from_data(
                std::format(CSS_FORMAT_RGB, Board::BACKGROUND_DEFAULT));
#ifdef WIN32
            picture.set_visible(false);
#endif
            break;
        }
    }
}

const std::string& ui::BoardWidget::get_background() const {
    return board->get_background();
}

void ui::BoardWidget::set_name(const std::string& board_name) {
    if (board) {
        board->set_name(board_name);
    }
}

const std::string& ui::BoardWidget::get_name() const {
    return board->get_name();
}

bool ui::BoardWidget::get_on_scroll() const { return on_scroll; }

void ui::BoardWidget::set_on_scroll(bool scroll) { on_scroll = scroll; }

std::shared_ptr<Board> ui::BoardWidget::get_board() const { return board; }

void ui::BoardWidget::setup_auto_scrolling() {
    auto drop_controller_motion_c = Gtk::DropControllerMotion::create();

    drop_controller_motion_c->signal_motion().connect(
        [this](double x, double y) {
            this->x = x;
            this->y = y;
        });
#ifdef WIN32
    scr.add_controller(drop_controller_motion_c);
#else
    add_controller(drop_controller_motion_c);
#endif
    Glib::signal_timeout().connect(
        [this]() {
#ifdef WIN32
            double cur_max_width = scr.get_width();
            auto hadjustment = scr.get_hadjustment();
#else
            double cur_max_width = get_width();
            auto hadjustment = get_hadjustment();
#endif
            double lower = hadjustment->get_lower();
            double upper = hadjustment->get_upper();

            if (on_scroll) {
                if (x >= (cur_max_width * 0.8)) {
                    double new_value =
                        hadjustment->get_value() + SCROLL_SPEED_FACTOR;
                    hadjustment->set_value(new_value >= upper ? upper
                                                              : new_value);
                } else if (x <= (cur_max_width * 0.2)) {
                    double new_value =
                        hadjustment->get_value() - SCROLL_SPEED_FACTOR;
                    hadjustment->set_value(new_value <= lower ? lower
                                                              : new_value);
                }
            }
            return true;
        },
        10);
}
