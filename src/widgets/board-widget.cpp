#include "board-widget.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <utils.h>
#include <window.h>

#include <format>

#include "cardlist-widget.h"
#include "core/colorable.h"

#ifdef WIN32
ui::BoardWidget::BoardWidget(BoardManager& manager)
    : Gtk::ScrolledWindow{},
      m_root{Gtk::Orientation::HORIZONTAL},
      m_manager{manager},
      picture{},
      scr{},
      overlay{},
      m_add_button{_("Add List")},
      m_css_provider{Gtk::CssProvider::create()} {
    set_child(overlay);
    picture.set_keep_aspect_ratio(false);

    overlay.set_child(picture);
    overlay.add_overlay(scr);
    overlay.set_expand(true);

    scr.set_child(m_root);

    Gtk::Widget::set_name("board-root");

    __setup_auto_scrolling();

    m_root.set_halign(Gtk::Align::START);
    m_root.set_spacing(25);
    m_root.set_margin(10);

    m_add_button.signal_clicked().connect(
        [this]() { add_new_cardlist(CardList{_("New CardList")}, false); });
    m_add_button.set_valign(Gtk::Align::START);
    m_add_button.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH);
    m_add_button.add_css_class("opaque");

    m_root.append(m_add_button);
}
#else
ui::BoardWidget::BoardWidget(BoardManager& manager)
    : Gtk::ScrolledWindow{},
      m_root{Gtk::Orientation::HORIZONTAL},
      m_manager{manager},
      m_add_button{_("Add List")},
      m_css_provider{Gtk::CssProvider::create()} {
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), m_css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    m_css_provider->load_from_data(BOARD_BACKGROUND);

    set_child(m_root);

    Gtk::Widget::set_name("board-root");

    __setup_auto_scrolling();

    m_root.set_halign(Gtk::Align::START);
    m_root.set_spacing(25);
    m_root.set_margin(10);

    m_add_button.signal_clicked().connect(
        [this]() { add_new_cardlist(CardList{_("New CardList")}, false); });
    m_add_button.set_valign(Gtk::Align::START);
    m_add_button.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH);
    m_add_button.add_css_class("opaque");

    m_root.append(m_add_button);
}
#endif

void ui::BoardWidget::set(const std::shared_ptr<Board>& board) {
    if (board) {
        this->m_board = board;
        __set_background(board->get_background());

        m_connections.emplace_back(board->signal_background().connect(
            sigc::mem_fun(*this, &BoardWidget::__set_background)));
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

void ui::BoardWidget::append(CardlistWidget& child) {
    m_cardlists.push_back(&child);

    m_root.append(child);
    m_root.reorder_child_after(m_add_button, child);

    add_cardlist_signal.emit(&child);
}

void ui::BoardWidget::reorder(CardlistWidget& next, CardlistWidget& sibling) {
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

            spdlog::get("app")->info(
                "Reordered Card list (\"{}\") before Card list (\"{}\")",
                next.cardlist()->get_name(), sibling.cardlist()->get_name());
            break;
        }
        case ReorderingType::UPDOWN: {
            m_root.reorder_child_after(next, sibling);
            spdlog::get("app")->info(
                "Reordered Card list (\"{}\") after Card list (\"{}\")",
                next.cardlist()->get_name(), sibling.cardlist()->get_name());
            break;
        }
        case ReorderingType::INVALID: {
            spdlog::get("app")->warn(
                "[BoardWidget.reorder] Cannot reorder (\"{}\") and (\"{}\")",
                next.cardlist()->get_name(), sibling.cardlist()->get_name());
            break;
        }
    }
}

void ui::BoardWidget::remove(ui::CardlistWidget& cardlist) {
    spdlog::get("app")->info("Removed CardList (\"{}\")",
                             cardlist.cardlist()->get_name());

    Gtk::Widget* prev_clist = cardlist.get_prev_sibling();
    Gtk::Widget* next_clist = cardlist.get_next_sibling();
    if (prev_clist) {
        prev_clist->grab_focus();
    } else if (next_clist && !G_TYPE_CHECK_INSTANCE_TYPE(
                                 next_clist->gobj(), Gtk::Button::get_type())) {
        next_clist->grab_focus();
    }

    m_root.remove(cardlist);
    std::erase(m_cardlists, &cardlist);

    m_board->container().remove(*cardlist.cardlist());
}

void ui::BoardWidget::clear() {
    std::for_each(m_connections.begin(), m_connections.end(),
                  [](auto& connection) { connection.disconnect(); });
    m_connections.clear();
    m_board = nullptr;
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

ui::CardlistWidget* ui::BoardWidget::add_new_cardlist(const CardList& cardlist,
                                                      bool editing_mode) {
    spdlog::get("app")->info("Added Card list (\"{}\")", cardlist.get_name());
    return __add_cardlist(m_board->container().append(cardlist), editing_mode);
}

ui::CardlistWidget* ui::BoardWidget::pop() {
    if (m_cardlists.empty()) {
        return nullptr;
    }

    auto cardlist = m_cardlists.back();
    m_root.remove(*cardlist);
    m_cardlists.pop_back();
    return cardlist;
}

ui::CardlistWidget* ui::BoardWidget::insert_new_cardlist_after(
    const CardList& cardlist, ui::CardlistWidget* sibling) {
    auto new_cardlist = Gtk::make_managed<ui::CardlistWidget>(
        *this,
        m_board->container().insert_after(cardlist, *sibling->cardlist()),
        false);
    m_cardlists.push_back(new_cardlist);
    m_root.insert_child_after(*new_cardlist, *sibling);

    add_cardlist_signal.emit(new_cardlist);

    return new_cardlist;
}

std::string ui::BoardWidget::get_background() const {
    return m_board->get_background();
}

std::string ui::BoardWidget::get_name() const { return m_board->get_name(); }

bool ui::BoardWidget::scroll() const { return m_on_scroll; }

bool ui::BoardWidget::empty() const { return m_cardlists.empty(); }

std::shared_ptr<Board> ui::BoardWidget::board() const { return m_board; }

sigc::signal<void(ui::CardlistWidget*)>&
ui::BoardWidget::signal_cardlist_added() {
    return add_cardlist_signal;
}

sigc::signal<void(ui::CardlistWidget*)>&
ui::BoardWidget::signal_cardlist_removed() {
    return remove_cardlist_signal;
}

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

// FIXME: Unstable background setting on Windows
void ui::BoardWidget::__set_background(const std::string& background) {
    BackgroundType bg_type = Board::get_background_type(background);
    switch (bg_type) {
        case BackgroundType::COLOR: {
            picture.set_visible(false);
            break;
        }
        case BackgroundType::IMAGE: {
            picture.set_filename(compressed_bg_filename(background));
            picture.set_visible(true);
            break;
        }
        case BackgroundType::INVALID: {
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
                std::format(BOARD_BACKGROUND_RGB, background));
            break;
        }
        case BackgroundType::IMAGE: {
            m_css_provider->load_from_data(std::format(
                BOARD_BACKGROUND_IMAGE, compressed_bg_filename(background)));
            break;
        }
        case BackgroundType::INVALID: {
            m_css_provider->load_from_data(
                std::format(BOARD_BACKGROUND_RGB, Board::BACKGROUND_DEFAULT));
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

    add_cardlist_signal.emit(new_cardlist);

    return new_cardlist;
}
