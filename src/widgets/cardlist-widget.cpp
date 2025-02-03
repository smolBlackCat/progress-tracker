#include "cardlist-widget.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>

#include "board-widget.h"
#include "card.h"
#include "glib.h"

extern "C" {
static void cardlist_class_init(void* klass, void* user_data) {
    g_return_if_fail(GTK_IS_WIDGET_CLASS(klass));
    gtk_widget_class_set_css_name(GTK_WIDGET_CLASS(klass), "cardlist");
}

static void cardlist_init(GTypeInstance* instance, void* klass) {
    g_return_if_fail(GTK_IS_WIDGET(instance));

    gtk_widget_set_focusable(GTK_WIDGET(instance), TRUE);
    gtk_widget_set_receives_default(GTK_WIDGET(instance), TRUE);
}
}

struct CardlistPayload {
    ui::CardlistWidget* cardlist_widget;
};

ui::CardlistInit::CardlistInit()
    : Glib::ExtraClassInit(cardlist_class_init, nullptr, cardlist_init) {}

ui::CardlistWidget::CardlistWidget(BoardWidget& board,
                                   std::shared_ptr<CardList> cardlist_refptr,
                                   bool is_new)
    : Glib::ObjectBase{"CardlistWidget"},
      CardlistInit{},
      Gtk::Widget{},
      add_card_button{_("Add card")},
      root{Gtk::Orientation::VERTICAL},
      card_widgets{},
      board{board},
      cardlist{cardlist_refptr},
      is_new{is_new},
      cardlist_header{cardlist_refptr->get_name(), "title-2", "title-2"},
      scr_window{} {
    set_layout_manager(Gtk::BoxLayout::create(Gtk::Orientation::VERTICAL));
    add_css_class("cardlist");
    set_halign(Gtk::Align::START);
    set_size_request(CARDLIST_MAX_WIDTH, -1);
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
    cardlist_header.set_margin_bottom(15);

    add_card_button.set_valign(Gtk::Align::CENTER);
    add_card_button.set_hexpand(true);
    add_card_button.signal_clicked().connect(
        [this]() { this->add(Card{_("New Card")}, true); });
    root.append(add_card_button);

    cardlist_header.insert_at_start(*this);

    for (auto& card : cardlist_refptr->get_cards()) {
        _add(card);
    }

    root.set_vexpand();
    root.set_spacing(15);

    scr_window.set_child(root);
    scr_window.set_size_request(CARDLIST_MAX_WIDTH, -1);
    scr_window.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scr_window.insert_at_end(*this);

    auto shortcut_controller = Gtk::ShortcutController::create();
    shortcut_controller->set_scope(Gtk::ShortcutScope::LOCAL);
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>N"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                auto n_cardlist =
                    this->board.add_cardlist(CardList{_("New Cardlist")}, true);

                this->board.reorder_cardlist(*n_cardlist, *this);
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Delete"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                this->board.remove_cardlist(*this);
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Left"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                CardlistWidget* previous_cardlist =
                    static_cast<CardlistWidget*>(this->get_prev_sibling());

                if (previous_cardlist) {
                    this->board.reorder_cardlist(*previous_cardlist, *this);
                }
                return true;
            })));
    shortcut_controller->add_shortcut(Gtk::Shortcut::create(
        Gtk::ShortcutTrigger::parse_string("<Control>Right"),
        Gtk::CallbackAction::create(
            [this](Gtk::Widget&, const Glib::VariantBase&) {
                Widget* maybe_cardlist = this->get_next_sibling();

                if (!G_TYPE_CHECK_INSTANCE_TYPE(maybe_cardlist->gobj(),
                                                Gtk::Button::get_type())) {
                    CardlistWidget* cardlist_widget =
                        static_cast<CardlistWidget*>(maybe_cardlist);
                    this->board.reorder_cardlist(*this, *cardlist_widget);
                }
                return true;
            })));
    add_controller(shortcut_controller);

    auto drop_motion_controller = Gtk::DropControllerMotion::create();
    drop_motion_controller->signal_motion().connect([this](double x, double y) {
        this->add_css_class("cardlist-to-drop");
    });
    drop_motion_controller->signal_leave().connect(
        [this]() { this->remove_css_class("cardlist-to-drop"); });
    add_controller(drop_motion_controller);

    signal_destroy().connect(
        sigc::mem_fun(cardlist_header, &Gtk::Widget::unparent));
    signal_destroy().connect(sigc::mem_fun(scr_window, &Gtk::Widget::unparent));
}

ui::CardlistWidget::~CardlistWidget() {
    if (!gobj()) {
        return;
    }

    cardlist_header.unparent();
    scr_window.unparent();
}

void ui::CardlistWidget::reorder(ui::CardWidget& next,
                                 ui::CardWidget& sibling) {
    root.reorder_child_after(next, sibling);
    cardlist->reorder(*next.get_card(), *sibling.get_card());

    spdlog::get("ui")->debug(
        "CardWidget \"{}\" has been reordered after CardWidget \"{}\"",
        next.get_card()->get_name(), sibling.get_card()->get_name());
}

const std::vector<ui::CardWidget*>& ui::CardlistWidget::get_card_widgets() {
    return card_widgets;
}

void ui::CardlistWidget::setup_drag_and_drop() {
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->signal_prepare().connect(
        [this, drag_source_c](double x, double y) {
            Glib::Value<CardlistPayload> value_cardlist_p;
            value_cardlist_p.init(Glib::Value<CardlistPayload>::value_type());
            value_cardlist_p.set(CardlistPayload{this});
            auto cardlist_icon = Gtk::WidgetPaintable::create(*this);
            drag_source_c->set_icon(cardlist_icon, x, y);
            return Gdk::ContentProvider::create(value_cardlist_p);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag) {
            this->set_opacity(0.5);
            this->board.set_on_scroll();

            spdlog::get("ui")->debug(
                "CardlistWidget \"{}\" has started dragging",
                this->get_cardlist()->get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag,
               Gdk::DragCancelReason reason) {
            this->set_opacity(1);
            this->board.set_on_scroll(false);

            spdlog::get("ui")->debug(
                "CardlistWidget \"{}\" has cancelled dragging",
                this->get_cardlist()->get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag, bool delete_data) {
            this->set_opacity(1);
            this->board.set_on_scroll(false);

            spdlog::get("ui")->debug("CardlistWidget \"{}\" has ended dragging",
                                     this->get_cardlist()->get_name());
        });
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    cardlist_header.add_controller(drag_source_c);

    auto drop_target_cardlist = Gtk::DropTarget::create(
        Glib::Value<CardlistPayload>::value_type(), Gdk::DragAction::MOVE);
    drop_target_cardlist->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->board.set_on_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<CardlistPayload>::value_type())) {
                Glib::Value<CardlistPayload> dropped_value;
                dropped_value.init(value.gobj());

                ui::CardlistWidget* dropped_cardlist =
                    dropped_value.get().cardlist_widget;

                if (dropped_cardlist == this) {
                    spdlog::get("ui")->warn(
                        "CardlistWidget \"{}\" has been dropped on itself. "
                        "Nothing happens",
                        this->get_cardlist()->get_name());
                    this->remove_css_class("cardlist-to-drop");
                    return true;
                }

                this->board.reorder_cardlist(*dropped_cardlist, *this);
                dropped_cardlist->set_opacity(1);

                this->remove_css_class("cardlist-to-drop");

                spdlog::get("ui")->debug(
                    "CardlistWidget \"{}\" has been dropped on CardlistWidget "
                    "\"{}\"",
                    dropped_cardlist->get_cardlist()->get_name(),
                    this->get_cardlist()->get_name());
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_cardlist);

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
                    this->add(*card_in_dropped);

                    spdlog::get("ui")->debug(
                        "CardWidget \"{}\" has been dropped on CardlistWidget "
                        "\"{}\"",
                        dropped_card->get_card()->get_name(),
                        this->get_cardlist()->get_name());
                }

                this->remove_css_class("cardlist-to-drop");
                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_card);
}

void ui::CardlistWidget::remove(ui::CardWidget* card) {
    spdlog::get("ui")->debug(
        "CardWidget \"{}\" has been removed from CardlistWidget \"{}\"",
        card->get_card()->get_name(), cardlist->get_name());

    root.remove(*card);
    cardlist->remove(*card->get_card());
    std::erase(card_widgets, card);
}

ui::CardWidget* ui::CardlistWidget::add(const Card& card, bool editing_mode) {
    return _add(cardlist->add(card), editing_mode);
}

const std::shared_ptr<CardList>& ui::CardlistWidget::get_cardlist() {
    return cardlist;
}

bool ui::CardlistWidget::is_child(ui::CardWidget* card) {
    for (auto& card_ : card_widgets) {
        if (card == card_) return true;
    }
    return false;
}

ui::CardWidget* ui::CardlistWidget::_add(const std::shared_ptr<Card>& card,
                                         bool editing_mode) {
    auto cardwidget = Gtk::make_managed<CardWidget>(card, editing_mode);
    card_widgets.push_back(cardwidget);
    cardwidget->set_cardlist(this);
    root.append(*cardwidget);
    root.reorder_child_after(add_card_button, *cardwidget);

    spdlog::get("ui")->debug(
        "CardWidget \"{}\" has been added to CardlistWidget \"{}\"",
        card->get_name(), cardlist->get_name());

    return cardwidget;
}

int ui::CardlistWidget::get_n_visible_children() const {
    // We could simply return 2 since the cardlist widget won't make their child
    // invisible at some point in the program
    int n_children = 0;
    for (const Widget* child = get_first_child(); child;
         child = child->get_next_sibling()) {
        if (child->get_visible()) ++n_children;
    }
    return n_children;
}

Gtk::SizeRequestMode ui::CardlistWidget::get_request_mode_vfunc() {
    return Gtk::SizeRequestMode::HEIGHT_FOR_WIDTH;
}

void ui::CardlistWidget::measure_vfunc(Gtk::Orientation orientation,
                                       int for_size, int& minimum, int& natural,
                                       int& minimum_baseline,
                                       int& natural_baseline) const {
    // Don't use baseline alignment.
    minimum_baseline = -1;
    natural_baseline = -1;

    minimum = 0;
    natural = 0;

    // Number of visible children.
    const int nvis_children = get_n_visible_children();

    if (orientation == Gtk::Orientation::HORIZONTAL) {
        // Divide the height equally among the visible children.
        if (for_size > 0 && nvis_children > 0) for_size /= nvis_children;

        // Request a width equal to the width of the widest visible child.
    }

    for (const Widget* child = get_first_child(); child;
         child = child->get_next_sibling())
        if (child->get_visible()) {
            int child_minimum, child_natural, ignore;
            child->measure(orientation, for_size, child_minimum, child_natural,
                           ignore, ignore);
            minimum = std::max(minimum, child_minimum);
            natural = std::max(natural, child_natural);
        }

    if (orientation == Gtk::Orientation::VERTICAL) {
        // The allocated height will be divided equally among the visible
        // children. Request a height equal to the number of visible
        // children times the height of the highest child.
        minimum *= nvis_children;
        natural *= nvis_children;
    }
}

void ui::CardlistWidget::size_allocate_vfunc(int width, int height,
                                             int baseline) {
    // Do something with the space that we have actually been given:
    //(We will not be given heights or widths less than we have requested,
    // though we might get more.)

    // Number of visible children.
    const int nvis_children = get_n_visible_children();

    if (nvis_children <= 0) {
        // No visible child.
        return;
    }

    // Assign space to the children:
    Gtk::Allocation child_allocation;
    const int height_per_child = height / nvis_children;

    // Place the first visible child at the top-left:
    child_allocation.set_x(0);
    child_allocation.set_y(0);

    // Make it take up the full width available:
    child_allocation.set_width(width);
    child_allocation.set_height(height_per_child);

    // Divide the height equally among the visible children.
    for (Widget* child = get_first_child(); child;
         child = child->get_next_sibling())
        if (child->get_visible()) {
            child->size_allocate(child_allocation, baseline);
            child_allocation.set_y(child_allocation.get_y() + height_per_child);
        }
}