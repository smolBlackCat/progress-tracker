#include "board-widget.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <window.h>

#include <format>

#include "cardlist-widget.h"
#include "core/colorable.h"

/**
 * TODO: High memory is allocated in setting background, mainly when the
 * background image is high. Should I try to compress it?
 */
ui::BoardWidget::BoardWidget(BoardManager& manager)
    : Gtk::ScrolledWindow{},
      m_root{Gtk::Orientation::HORIZONTAL},
      m_manager{manager},
#ifdef WIN32
      picture{},
      scr{},
      overlay{},
#endif
      m_add_button{_("Add List")},
      m_css_provider{Gtk::CssProvider::create()} {

#ifdef WIN32
    set_child(overlay);
    picture.set_keep_aspect_ratio(false);
    overlay.set_child(picture);
    scr.set_child(m_root);
    overlay.add_overlay(scr);
    overlay.set_expand(true);
#else
    set_child(m_root);
#endif
    Gtk::Widget::set_name("board-root");

    __setup_auto_scrolling();

    m_root.set_halign(Gtk::Align::START);
    m_root.set_spacing(25);
    m_root.set_margin(10);

    Gtk::StyleProvider::add_provider_for_display(
        get_display(), m_css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    m_css_provider->load_from_data(CSS_FORMAT);

    m_add_button.signal_clicked().connect(
        [this]() { add_cardlist(CardList{_("New CardList")}, true); });
    m_add_button.set_valign(Gtk::Align::START);
    m_add_button.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH);
    m_add_button.add_css_class("opaque");

    m_root.append(m_add_button);

    // Auto-saves the Board after 10 secs
    Glib::signal_timeout().connect(
        [this]() {
            if (this->m_board) {
                this->save(false);
            }
            return true;
        },
        BoardWidget::SAVE_INTERVAL);

    // Update due date labels of every card in the board every minute
    Glib::signal_timeout().connect(
        [this]() {
            if (this->m_board) {
                for (auto& cardlist : this->m_cardlists) {
                    for (auto& card : cardlist->cards()) {
                        card->set_tooltip_markup(card->create_details_text());
                        card->update_due_date_label();
                    }
                }
            }
            return true;
        },
        BoardWidget::UPDATE_INTERVAL);
}

void ui::BoardWidget::set(const std::shared_ptr<Board>& board) {
    if (board) {
        this->m_board = board;

        __set_background(board->get_background());
        for (auto& cardlist : board->container()) {
            __add_cardlist(cardlist, false);
        }

        m_connections.emplace_back(board->signal_background().connect(
            sigc::mem_fun(*this, &BoardWidget::__set_background)));

        spdlog::get("ui")->debug("[BoardWidget] Board \"{}\" has been set",
                                 board->get_name());
    } else {
        spdlog::get("ui")->warn(
            "[BoardWidget] Board or BoardCardButton is "
            "not set properly");
    }
}

void ui::BoardWidget::set_name(const std::string& board_name) {
    if (m_board) {
        m_board->set_name(board_name);
    }
}

void ui::BoardWidget::set_background(const std::string& background) {
    BackgroundType bg_type = Board::get_background_type(background);
    if (bg_type == BackgroundType::COLOR) {
        m_board->set_background(string_to_color(background));
    } else if (bg_type == BackgroundType::IMAGE) {
        m_board->set_background(background);
    }
}

void ui::BoardWidget::reorder_cardlist(CardlistWidget& next,
                                       CardlistWidget& sibling) {
    ReorderingType reordering =
        m_board->container().reorder(*next.cardlist(), *sibling.cardlist());

    switch (reordering) {
        case ReorderingType::DOWNUP: {
            auto sibling_sibling = sibling.get_prev_sibling();
            if (!sibling_sibling) {
                m_root.reorder_child_at_start(next);
            } else {
                m_root.reorder_child_after(next, *sibling_sibling);
            }

            spdlog::get("ui")->debug(
                "[BoardWidget] CardListWidget \"{}\" was inserted before "
                "CardListWidget \"{}\"",
                next.cardlist()->get_name(), sibling.cardlist()->get_name());
            break;
        }
        case ReorderingType::UPDOWN: {
            m_root.reorder_child_after(next, sibling);
            spdlog::get("ui")->debug(
                "[BoardWidget] CardListWidget \"{}\" was inserted after "
                "CardListWidget \"{}\"",
                next.cardlist()->get_name(), sibling.cardlist()->get_name());
            break;
        }
        case ReorderingType::INVALID: {
            spdlog::get("ui")->warn("[BoardWidget] Cannot reorder cardlists:");
            break;
        }
    }
}

void ui::BoardWidget::remove_cardlist(ui::CardlistWidget& cardlist) {
    spdlog::get("ui")->debug(
        "[BoardWidget] CardlistWidget \"{}\" has been removed",
        cardlist.cardlist()->get_name());

    m_root.remove(cardlist);
    std::erase(m_cardlists, &cardlist);

    m_board->container().remove(*cardlist.cardlist());
}

void ui::BoardWidget::clear() {
    if (!m_cardlists.empty()) {
        for (const auto& cardlist_widget : m_cardlists) {
            m_root.remove(*cardlist_widget);
        }
        m_cardlists.clear();
    }

    std::for_each(m_connections.begin(), m_connections.end(),
                  [](auto& connection) { connection.disconnect(); });
    m_connections.clear();

    spdlog::get("ui")->info("[BoardWidget] Board view has been cleared");
}

void ui::BoardWidget::save(bool clear_after_save) {
    if (m_board->modified()) {
        m_manager.local_save(m_board);
    }

    if (clear_after_save) {
        clear();
    }
}

void ui::BoardWidget::set_scroll(bool scroll) { m_on_scroll = scroll; }

ui::CardlistWidget* ui::BoardWidget::add_cardlist(const CardList& cardlist,
                                                  bool editing_mode) {
    return __add_cardlist(m_board->container().append(cardlist), editing_mode);
}

ui::CardlistWidget* ui::BoardWidget::insert_new_cardlist_after(
    const CardList& cardlist, ui::CardlistWidget* sibling) {
    auto new_cardlist = Gtk::make_managed<ui::CardlistWidget>(
        *this,
        m_board->container().insert_after(cardlist, *sibling->cardlist()),
        true);
    m_cardlists.push_back(new_cardlist);

    m_root.insert_child_after(*new_cardlist, *sibling);

    spdlog::get("ui")->debug(
        "[BoardWidget] CardlistWidget \"{}\" has been added",
        cardlist.get_name());

    return new_cardlist;
}

std::string ui::BoardWidget::get_background() const {
    return m_board->get_background();
}

std::string ui::BoardWidget::get_name() const { return m_board->get_name(); }

bool ui::BoardWidget::scroll() const { return m_on_scroll; }

std::shared_ptr<Board> ui::BoardWidget::board() const { return m_board; }

void ui::BoardWidget::__setup_auto_scrolling() {
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

            if (m_on_scroll) {
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

#ifdef WIN32

void ui::BoardWidget::__set_background(const std::string& background) {
    BackgroundType bg_type = Board::get_background_type(background);
    switch (bg_type) {
        case BackgroundType::COLOR: {
            m_css_provider->load_from_data(
                std::format(CSS_FORMAT_RGB, background));
            picture.set_visible(false);
            break;
        }
        case BackgroundType::IMAGE: {
            picture.set_filename(background);
            picture.set_visible(true);
            break;
        }
        case BackgroundType::INVALID: {
            m_css_provider->load_from_data(
                std::format(CSS_FORMAT_RGB, Board::BACKGROUND_DEFAULT));
            picture.set_visible(false);
            break;
        }
    }
}
#else

void ui::BoardWidget::__set_background(const std::string& background) {
    BackgroundType bg_type = Board::get_background_type(background);
    switch (bg_type) {
        case BackgroundType::COLOR: {
            m_css_provider->load_from_data(
                std::format(CSS_FORMAT_RGB, background));
            break;
        }
        case BackgroundType::IMAGE: {
            m_css_provider->load_from_data(
                std::format(CSS_FORMAT_FILE, background));
            break;
        }
        case BackgroundType::INVALID: {
            m_css_provider->load_from_data(
                std::format(CSS_FORMAT_RGB, Board::BACKGROUND_DEFAULT));
            break;
        }
    }
}
#endif

ui::CardlistWidget* ui::BoardWidget::__add_cardlist(
    const std::shared_ptr<CardList>& cardlist, bool editing_mode) {
    auto new_cardlist =
        Gtk::make_managed<ui::CardlistWidget>(*this, cardlist, editing_mode);
    m_cardlists.push_back(new_cardlist);

    m_root.append(*new_cardlist);
    m_root.reorder_child_after(m_add_button, *new_cardlist);

    spdlog::get("ui")->debug(
        "[BoardWidget] CardlistWidget \"{}\" has been added",
        cardlist->get_name());

    return new_cardlist;
}