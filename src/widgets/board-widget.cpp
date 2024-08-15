#include <glibmm/i18n.h>
#include <window.h>

#include <format>

#include "board-widget.h"
#include "cardlist-widget.h"

/**
 * TODO: High memory is allocated in setting background, mainly when the
 * background image is high. Should I try to compress it?
 */
ui::BoardWidget::BoardWidget(ui::ProgressWindow& app_window)
    : Gtk::ScrolledWindow{},
      root{Gtk::Orientation::HORIZONTAL},
      add_button{_("Add List")},
#ifdef WIN32
      picture{},
      scr{},
      overlay{},
#endif
      app_window{app_window},
      on_drag{false} {

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
    set_name("board-root");

    setup_auto_scrolling();

    root.set_spacing(25);
    root.set_margin(10);

    css_provider_refptr = Gtk::CssProvider::create();
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), css_provider_refptr, GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_provider_refptr->load_from_data(CSS_FORMAT);

    add_button.signal_clicked().connect(
        [this]() { add_cardlist(CardList{_("New CardList")}, true); });
    add_button.set_halign(Gtk::Align::START);
    add_button.set_valign(Gtk::Align::START);
    add_button.set_hexpand();
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
}

ui::BoardWidget::~BoardWidget() {}

void ui::BoardWidget::set(Board* board, BoardCardButton* board_card_button) {
    if (board && board_card_button) {
        clear();
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
        success = board->save_as_xml();
    }
    board_card_button->update(board);
    if (free) {
        delete board;
        board = nullptr;
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
    board->remove_cardlist(*cardlist.get_cardlist_refptr());
    return true;
}

void ui::BoardWidget::reorder_cardlist(CardlistWidget& next,
                                       CardlistWidget& sibling) {
    board->reorder_cardlist(next.get_cardlist_refptr(),
                            sibling.get_cardlist_refptr());
    root.reorder_child_after(next, sibling);
}

void ui::BoardWidget::set_background(const std::string& background,
                                     bool modify) {
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

