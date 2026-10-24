#include "board-widget.h"

#include <glibmm/i18n.h>
#include <utils.h>
#include <window.h>

#include <format>

#include "cardlist-widget.h"

namespace ui {
#ifdef WIN32
BoardWidget::BoardWidget()
    : Gtk::ScrolledWindow{},
      m_root{Gtk::Orientation::HORIZONTAL},
      m_picture{},
      m_scr{},
      m_overlay{},
      m_add_button{_("Add List")},
      m_css_provider{Gtk::CssProvider::create()} {
    Gtk::StyleProvider::add_provider_for_display(
        get_display(), m_css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    m_css_provider->load_from_data(BOARD_BACKGROUND);
    set_child(m_overlay);
    m_picture.set_keep_aspect_ratio(false);

    m_overlay.set_child(m_picture);
    m_overlay.add_overlay(m_scr);
    m_overlay.set_expand(true);

    m_scr.set_child(m_root);

    Gtk::Widget::set_name("board-root");

    __setup_auto_scrolling();

    m_root.set_halign(Gtk::Align::START);
    m_root.set_spacing(25);
    m_root.set_margin(10);

    m_add_button.signal_clicked().connect([this]() {
        CardlistWidget* new_cardlist =
            Gtk::make_managed<CardlistWidget>(*this, _("New CardList"));
        append(*new_cardlist);
    });
    m_add_button.set_valign(Gtk::Align::START);
    m_add_button.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH);
    m_add_button.add_css_class("opaque");

    m_root.append(m_add_button);

    m_container_adapter.signal_item_added().connect(m_cardlist_added_signal);
    m_container_adapter.signal_item_removed().connect(m_cardlist_remove_signal);
    m_container_adapter.signal_item_reordered().connect(
        m_cardlist_reorder_signal);
}
#else
BoardWidget::BoardWidget()
    : Gtk::ScrolledWindow{},
      m_root{Gtk::Orientation::HORIZONTAL},
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

    m_add_button.signal_clicked().connect([this]() {
        CardlistWidget* new_cardlist =
            Gtk::make_managed<CardlistWidget>(*this, _("New CardList"));
        append(*new_cardlist, true);
    });

    m_add_button.set_valign(Gtk::Align::START);
    m_add_button.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH);
    m_add_button.add_css_class("opaque");

    m_root.append(m_add_button);

    m_container_adapter.signal_item_added().connect(m_cardlist_added_signal);
    m_container_adapter.signal_item_removed().connect(m_cardlist_remove_signal);
    m_container_adapter.signal_item_reordered().connect(
        m_cardlist_reorder_signal);
}
#endif

void BoardWidget::set_name(const std::string& board_name) {
    m_name_changed_signal.emit(m_name, board_name);
    m_name = board_name;
}

#ifdef WIN32
void BoardWidget::set_background(const std::string& background) {
    BackgroundType bg_type = Board::get_background_type(background);
    switch (bg_type) {
        case BackgroundType::COLOR: {
            m_background_changed_signal.emit(m_background, background);
            m_background = background;
            m_css_provider->load_from_data(
                std::format(BOARD_BACKGROUND_RGB, background));
            m_picture.set_visible(false);
            break;
        }
        case BackgroundType::IMAGE: {
            m_background_changed_signal.emit(m_background, background);
            m_background = background;
            m_picture.set_filename(compressed_bg_filename(background));
            m_picture.set_visible(true);
            break;
        }
        case BackgroundType::INVALID: {
            m_picture.set_visible(false);
            break;
        }
    }
}
#else
void BoardWidget::set_background(const std::string& background) {
    BackgroundType bg_type = Board::get_background_type(background);
    switch (bg_type) {
        case BackgroundType::COLOR: {
            m_background_changed_signal.emit(m_background, background);
            m_background = background;
            m_css_provider->load_from_data(
                std::format(BOARD_BACKGROUND_RGB, background));
            break;
        }
        case BackgroundType::IMAGE: {
            m_background_changed_signal.emit(m_background, background);
            m_background = background;
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

void BoardWidget::append(CardlistWidget& child, bool interactive) {
    m_container_adapter.append(child);

    if (interactive) {
        Glib::signal_idle().connect_once(
            [this, &child]() {
                Gtk::ScrolledWindow* vp = static_cast<Gtk::ScrolledWindow*>(
                    m_root.get_parent()->get_parent());
                child.grab_focus();
                auto hadj = vp->get_hadjustment();
                hadj->set_value(hadj->get_upper() + 1);
            },
            Glib::PRIORITY_LOW);
    }
}

void BoardWidget::insert_after(CardlistWidget& widget,
                               CardlistWidget& sibling) {
    m_container_adapter.insert_after(widget, sibling);

    widget.grab_focus();

#if GTKMM_CHECK_VERSION(4, 12, 0)
    static_cast<Gtk::Viewport*>(m_root.get_parent())
        ->scroll_to(*widget.get_next_sibling());
#else
    gtk_viewport_scroll_to(GTK_VIEWPORT(m_root.get_parent()->gobj()),
                           widget.get_next_sibling()->gobj(), NULL);
#endif
}

void BoardWidget::reorder_after(CardlistWidget& next, CardlistWidget& sibling) {
    m_container_adapter.reorder_after(next, sibling);
#if GTKMM_CHECK_VERSION(4, 12, 0)
    static_cast<Gtk::Viewport*>(m_root.get_parent())->scroll_to(sibling);
#else
    gtk_viewport_scroll_to(GTK_VIEWPORT(m_root.get_parent()->gobj()),
                           sibling.gobj(), NULL);
#endif
}
void BoardWidget::reorder_before(CardlistWidget& next,
                                 CardlistWidget& sibling) {
    m_container_adapter.reorder_before(next, sibling);
#if GTKMM_CHECK_VERSION(4, 12, 0)
    static_cast<Gtk::Viewport*>(m_root.get_parent())->scroll_to(sibling);
#else
    gtk_viewport_scroll_to(GTK_VIEWPORT(m_root.get_parent()->gobj()),
                           sibling.gobj(), NULL);
#endif
}

void BoardWidget::remove(CardlistWidget& cardlist) {
    if (Gtk::Widget* previous_sibling = cardlist.get_prev_sibling()) {
        previous_sibling->grab_focus();
    } else {
        cardlist.get_next_sibling()->grab_focus();
    }

    m_container_adapter.remove(cardlist);
}

void BoardWidget::set_scroll(bool scroll) {
    m_on_scroll = scroll;
    m_scroll_changed_signal.emit();
}

// FIXME: The application of this method must be reviewed
void BoardWidget::pop() {
    if (Gtk::Widget* cardlist = m_add_button.get_prev_sibling()) {
        m_cardlist_remove_signal.emit(static_cast<CardlistWidget*>(cardlist));
        m_root.remove(*cardlist);
    }
}

const std::string& BoardWidget::get_background() const { return m_background; }

const std::string& BoardWidget::get_name() const { return m_name; }

bool BoardWidget::scroll() const { return m_on_scroll; }

bool BoardWidget::empty() const { return !m_add_button.get_prev_sibling(); }

sigc::signal<void(std::string, std::string)>&
BoardWidget::signal_name_changed() {
    return m_name_changed_signal;
}

sigc::signal<void(std::string, std::string)>&
BoardWidget::signal_background_changed() {
    return m_background_changed_signal;
}

sigc::signal<void(CardlistWidget*, int)>& BoardWidget::signal_added_cardlist() {
    return m_cardlist_added_signal;
}

sigc::signal<void(CardlistWidget*)>& BoardWidget::signal_remove_cardlist() {
    return m_cardlist_remove_signal;
}

sigc::signal<void(CardlistWidget*, CardlistWidget*, bool)>&
BoardWidget::signal_reorder() {
    return m_cardlist_reorder_signal;
}

void BoardWidget::__setup_auto_scrolling() {
    auto drop_controller_motion_c = Gtk::DropControllerMotion::create();

    drop_controller_motion_c->signal_motion().connect(
        [this](double x, double y) {
            this->m_x = x;
            this->m_y = y;
        });
#ifdef WIN32
    m_scr.add_controller(drop_controller_motion_c);
#else
    add_controller(drop_controller_motion_c);
#endif

    m_scroll_changed_signal.connect([this]() {
        if (m_on_scroll) {
            m_timeout_scroller = Glib::signal_timeout().connect(
                [this]() {
#ifdef WIN32
                    double cur_max_width = m_scr.get_width();
                    auto hadjustment = m_scr.get_hadjustment();
#else
                    double cur_max_width = get_width();
                    auto hadjustment = get_hadjustment();
#endif
                    double lower = hadjustment->get_lower();
                    double upper = hadjustment->get_upper();

                    if (m_x >= (cur_max_width * 0.8)) {
                        double new_value =
                            hadjustment->get_value() + SCROLL_SPEED_FACTOR;
                        hadjustment->set_value(new_value >= upper ? upper
                                                                  : new_value);
                    } else if (m_x <= (cur_max_width * 0.2)) {
                        double new_value =
                            hadjustment->get_value() - SCROLL_SPEED_FACTOR;
                        hadjustment->set_value(new_value <= lower ? lower
                                                                  : new_value);
                    }
                    return true;
                },
                10);
        } else {
            m_timeout_scroller.disconnect();
        }
    });
}
}  // namespace ui
