#include "card.h"

#include <dialog/card-dialog.h>
#include <glibmm/i18n.h>
#include <utils.h>

#include "cardlist-widget.h"

ui::CardWidget::CardWidget(std::shared_ptr<Card> card_refptr, bool is_new)
    : ui::EditableLabelHeader{card_refptr->get_name()},
      card_refptr{card_refptr},
      cardlist_p{nullptr},
      is_new{is_new},
      color_dialog{Gtk::ColorDialog::create()} {
    add_css_class("card");

    add_option_button(_("Card Details"), "card-details", [this]() {
        auto card_details_dialog = CardDetailsDialog::create(*this);
        card_details_dialog->set_transient_for(
            *(static_cast<Gtk::Window*>(get_root())));
        card_details_dialog->set_hide_on_close(false);
        card_details_dialog->set_modal();
        card_details_dialog->set_visible();
    });

    auto card_cover_submenu = Gio::Menu::create();
    auto card_cover_actions = Gio::SimpleActionGroup::create();

    card_cover_submenu->append(_("Set Color"), "cover.set-color");
    card_cover_submenu->append(_("Clear Color Frame"), "cover.clear-color");

    card_cover_actions->add_action(
        "set-color", sigc::mem_fun(*this, &ui::CardWidget::open_color_dialog));
    card_cover_actions->add_action(
        "clear-color", sigc::mem_fun(*this, &CardWidget::clear_color));
    menu_button.insert_action_group("cover", card_cover_actions);
    menu->append_submenu(_("Card Cover"), card_cover_submenu);

    add_option_button(
        _("Remove"), "remove",
        sigc::mem_fun(*this, &ui::CardWidget::remove_from_parent));

    color_frame.set_content_fit(Gtk::ContentFit::FILL);
    color_frame.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH,
                                 COLOR_FRAME_HEIGHT);

    m_frame.set_size_request(CardlistWidget::CARDLIST_MAX_WIDTH,
                             COLOR_FRAME_HEIGHT);
    m_frame.set_margin_bottom(4);
    m_frame.set_child(color_frame);
    m_frame.set_visible(false);

    append(m_frame);
    reorder_child_at_start(m_frame);

    if (card_refptr->is_color_set()) {
        Color card_color = card_refptr->get_color();
        set_color(Gdk::RGBA{static_cast<float>(std::get<0>(card_color)) / 255,
                            static_cast<float>(std::get<1>(card_color)) / 255,
                            static_cast<float>(std::get<2>(card_color)) / 255,
                            std::get<3>(card_color)});

        if (!is_new) {
            card_refptr->set_modified(false);
        }
    }

    signal_on_confirm().connect([this](const std::string& label) {
        this->card_refptr->set_name(label);
        this->is_new = false;
    });
    signal_on_cancel().connect([this](const std::string& label) {
        if (this->is_new) {
            this->remove_from_parent();
        }
    });

    set_tooltip_text(create_details_text());

    setup_drag_and_drop();

    if (card_refptr->get_tasks().size() == 0) {
        progress_bar.set_visible(false);
    }

    progress_bar.set_fraction(card_refptr->get_completion() / 100);
    append(progress_bar);
}

void ui::CardWidget::remove_from_parent() {
    if (cardlist_p) {
        cardlist_p->remove_card(this);
    }

    // The pixbuf in color_frame is invalid on drag and drops events.
    // Thus it is important to clean whatever existent instances.
    color_frame.set_pixbuf(nullptr);
}

void ui::CardWidget::set_cardlist(ui::CardlistWidget* cardlist_p) {
    if (cardlist_p) {
        this->cardlist_p = cardlist_p;
    }
}

std::shared_ptr<Card> ui::CardWidget::get_card() { return card_refptr; }

ui::CardlistWidget const* ui::CardWidget::get_cardlist_widget() const {
    return cardlist_p;
}

void ui::CardWidget::update_completed() {
    progress_bar.set_fraction(card_refptr->get_completion() / 100);
}

void ui::CardWidget::hide_progress_bar(bool hide) {
    progress_bar.set_visible(!hide);
}

void ui::CardWidget::setup_drag_and_drop() {
    // DragSource Settings
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    drag_source_c->signal_prepare().connect(
        [this, drag_source_c](double x, double y) {
            Glib::Value<ui::CardWidget*> value_new_cardptr;
            value_new_cardptr.init(Glib::Value<ui::CardWidget*>::value_type());
            value_new_cardptr.set(this);
            auto card_icon = Gtk::WidgetPaintable::create(*this);
            drag_source_c->set_icon(card_icon, x, y);
            return Gdk::ContentProvider::create(value_new_cardptr);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref) {
            this->cardlist_p->board.set_on_scroll();
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref,
               Gdk::DragCancelReason reason) {
            this->cardlist_p->board.set_on_scroll(false);
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref, bool s) {
            this->cardlist_p->board.set_on_scroll(false);
        });
    add_controller(drag_source_c);

    // DropTarget Settings
    auto drop_target_c = Gtk::DropTarget::create(
        Glib::Value<ui::CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_c->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->cardlist_p->board.set_on_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<ui::CardWidget*>::value_type())) {
                Glib::Value<ui::CardWidget*> dropped_value;
                dropped_value.init(value.gobj());
                auto dropped_card = dropped_value.get();

                if (dropped_card == this) {
                    return true;
                }

                if (this->cardlist_p->is_child(dropped_card)) {
                    this->cardlist_p->reorder_cardwidget(*dropped_card, *this);
                } else {
                    auto card_from_dropped = dropped_card->get_card();
                    CardWidget* dropped_copy =
                        this->cardlist_p->add_card(*card_from_dropped);
                    dropped_card->remove_from_parent();
                    this->cardlist_p->reorder_cardwidget(*dropped_copy, *this);
                }
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_c);
}

void ui::CardWidget::open_color_dialog() {
    color_dialog->choose_rgba(
        *static_cast<Gtk::Window*>(get_root()),
        [this](const Glib::RefPtr<Gio::AsyncResult>& result) {
            try {
                set_color(color_dialog->choose_rgba_finish(result));
            } catch (Gtk::DialogError& err) {
                // FIXME: This would be a good opportunity to have a logging
                // system here
            }
        });
}

void ui::CardWidget::clear_color() {
    m_frame.set_visible(false);
    card_refptr->set_color(NO_COLOR);
}

void ui::CardWidget::set_color(const Gdk::RGBA& color) {
    auto color_frame_pixbuf = Gdk::Pixbuf::create(
        Gdk::Colorspace::RGB, false, 8, CardlistWidget::CARDLIST_MAX_WIDTH, 30);
    card_refptr->set_color(Color{color.get_red() * 255, color.get_green() * 255,
                                 color.get_blue() * 255, 1.0});
    color_frame_pixbuf->fill(rgb_to_hex(card_refptr->get_color()));
    color_frame.set_pixbuf(color_frame_pixbuf);
    this->m_frame.set_visible();
}

std::string ui::CardWidget::create_details_text() const {
    std::ostringstream details_text;

    if (!card_refptr->get_tasks().empty()) {
        details_text << Glib::ustring::compose(_("%1%% complete"),
                                               card_refptr->get_completion())
                     << "\n\n";
    }

    if (!card_refptr->get_notes().empty()) {
        details_text << _("Notes") << ":\n"
                     << card_refptr->get_notes() << "\n\n";
    }

    std::string final_text = details_text.str();

    if (!final_text.empty()) {
        final_text.resize(final_text.size() - 2);
    }

    return final_text;
}
