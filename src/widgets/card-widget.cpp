#include "card-widget.h"

#include <dialog/card-dialog.h>
#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <window.h>

#include <numeric>

#include "cardlist-widget.h"
#include "core/colorable.h"

extern "C" {
static void card_class_init(void* g_class, void* data) {
    g_return_if_fail(GTK_IS_WIDGET_CLASS(g_class));
    gtk_widget_class_set_css_name(GTK_WIDGET_CLASS(g_class), "card");
}

static void card_init(GTypeInstance* instance, void* g_class) {
    g_return_if_fail(GTK_IS_WIDGET(instance));

    gtk_widget_set_focusable(GTK_WIDGET(instance), TRUE);
    gtk_widget_set_receives_default(GTK_WIDGET(instance), TRUE);
}
}

namespace ui {
CardInit::CardInit()
    : Glib::ExtraClassInit(card_class_init, nullptr, card_init) {}

std::string label_for_color(CoverColor color) {
    switch (color) {
        case CoverColor::UNSET:
            return _("Unset Color");
        case CoverColor::BLUE:
            return _("Blue");
        case CoverColor::RED:
            return _("Red");
        case CoverColor::ORANGE:
            return _("Orange");
        case CoverColor::GREEN:
            return _("Green");
        case CoverColor::YELLOW:
            return _("Yellow");
        case CoverColor::PURPLE:
            return _("Purple");
        default:
            return "Unknown";
    }
}

std::string cover_color_css(CoverColor color) {
    switch (color) {
        case CoverColor::UNSET:
            return "unset";
        case CoverColor::BLUE:
            return "blue";
        case CoverColor::RED:
            return "red";
        case CoverColor::ORANGE:
            return "orange";
        case CoverColor::GREEN:
            return "green";
        case CoverColor::YELLOW:
            return "yellow";
        case CoverColor::PURPLE:
            return "purple";
        default:
            return "unknown";
    }
}

std::string format_date_str(Date date) {
    const std::time_t time =
        std::chrono::system_clock::to_time_t(std::chrono::sys_days(date));

    char date_str[30];
    std::strftime(date_str, sizeof(date_str), "%d %b, %Y", std::gmtime(&time));

    return date_str;
}

CoverColor rgb_to_cover_color(Color rgb) {
    const std::map<Color, CoverColor> CARD_COLORS = {
        {NO_COLOR, CoverColor::UNSET},      {RED_COLOR, CoverColor::RED},
        {ORANGE_COLOR, CoverColor::ORANGE}, {YELLOW_COLOR, CoverColor::YELLOW},
        {GREEN_COLOR, CoverColor::GREEN},   {BLUE_COLOR, CoverColor::BLUE},
        {PURPLE_COLOR, CoverColor::PURPLE}};

    return CARD_COLORS.at(rgb);
}

std::string truncate(const std::string& text) {
    const int TRUNCATE_SIZE = 40;

    if (text.size() <= TRUNCATE_SIZE) {
        return text;
    }

    std::string truncated_text = text.substr(0, TRUNCATE_SIZE - 3);
    truncated_text += "...";

    return truncated_text;
}

std::string time_info_text(const Date& cur_date, const Date& card_due_date) {
    auto now = sys_days(cur_date);
    auto days = sys_days(card_due_date);
    auto delta_days = (days - now).count();

    std::string due_date_text;

    if (delta_days > 0 && delta_days < 30) {
        due_date_text =
            Glib::ustring::compose(Glib::locale_to_utf8(ngettext(
                                       "<b>%1 day</b> remaining",
                                       "<b>%1 days</b> remaining", delta_days)),
                                   delta_days) +
            "\n\n";
    } else if (delta_days > 0 && delta_days >= 30 && delta_days < 365) {
        long months_from_delta =
            delta_days / 30;  // Assume every month has 30 days
        due_date_text =
            Glib::ustring::compose(
                Glib::locale_to_utf8(ngettext("<b>%1 month</b> remaining",
                                              "<b>%1 months</b> remaining",
                                              months_from_delta)),
                months_from_delta) +
            "\n\n";
    } else if (delta_days > 0 && delta_days >= 365) {
        long years_from_delta = delta_days / 365;  // Ignore leap years
        due_date_text = Glib::ustring::compose(
                            Glib::locale_to_utf8(ngettext(
                                "<b>%1 year</b> remaining",
                                "<b>%1 years</b> remaining", years_from_delta)),
                            years_from_delta) +
                        "\n\n";
    } else if (delta_days > -30) {
        due_date_text = Glib::ustring::compose(
                            Glib::locale_to_utf8(ngettext(
                                "This card is past due date <b>%1 day ago</b>",
                                "This card is past due date <b>%1 days ago</b>",
                                -delta_days)),
                            -delta_days) +
                        "\n\n";
    } else if (delta_days <= -30 && delta_days > -365) {
        long months_from_delta =
            delta_days / 30;  // Assume every month has 30 days
        due_date_text =
            Glib::ustring::compose(
                Glib::locale_to_utf8(
                    ngettext("This card is past due date <b>%1 month</b> ago",
                             "This card is past due date <b>%1 months</b> "
                             "ago",
                             -months_from_delta)),
                -months_from_delta) +
            "\n\n";
    } else if (delta_days <= -365) {
        long years_from_delta = delta_days / 365;  // Ignore leap years
        due_date_text =
            Glib::ustring::compose(
                Glib::locale_to_utf8(
                    ngettext("This card is past due date <b>%1 year</b> ago",
                             "This card is past due date <b>%1 years</b> ago",
                             -years_from_delta)),
                -years_from_delta) +
            "\n\n";
    }

    return due_date_text;
}

const std::map<CoverColor, Gdk::RGBA> CardWidget::CARD_COLORS = {
    {CoverColor::UNSET, Gdk::RGBA{"rgba(0,0,0,0)"}},
    {CoverColor::RED, Gdk::RGBA{"rgb(165, 29, 45)"}},
    {CoverColor::ORANGE, Gdk::RGBA{"rgb(198, 70, 0)"}},
    {CoverColor::YELLOW, Gdk::RGBA{"rgb(229, 165, 10)"}},
    {CoverColor::GREEN, Gdk::RGBA{"rgb(38, 162, 105)"}},
    {CoverColor::BLUE, Gdk::RGBA{"rgb(26, 95, 180)"}},
    {CoverColor::PURPLE, Gdk::RGBA{"rgb(32, 9, 65)"}}};

std::map<CardWidget*, std::vector<CardWidget::CardPopover*>>
    CardWidget::CardPopover::registered_card_popovers{};

CardWidget::CardPopover::CardPopover(CardWidget* card_widget)
    : Gtk::PopoverMenu{}, m_card_widget{card_widget} {
    if (!registered_card_popovers.contains(card_widget)) {
        registered_card_popovers[card_widget] = std::vector<CardPopover*>{this};
    } else {
        registered_card_popovers[card_widget].push_back(this);
    }

    set_has_arrow(false);
    set_position(Gtk::PositionType::BOTTOM);

    using ButtonAction =
        std::tuple<const char*, const char*, std::function<void()>>;

    const std::array<ButtonAction, 3> button_actions = {
        ButtonAction{"rename", _("Rename"),
                     [this, card_widget] {
                         card_widget->on_rename();
                         this->popdown();
                     }},
        ButtonAction{"card-details", _("Card Details"),
                     [this, card_widget] {
                         card_widget->open_card_details_dialog();
                         this->popdown();
                     }},
        ButtonAction{"remove", _("Remove"), [this, card_widget] {
                         this->popdown();
                         card_widget->remove_from_parent();
                     }}};

    auto menu_model = Gio::Menu::create();
    auto action_group = Gio::SimpleActionGroup::create();

    for (const auto& [key, label, action] : button_actions) {
        menu_model->append(label, std::format("card-popover.{}", key));
        action_group->add_action(key, action);
    }

    auto color_buttons_menu = Gio::MenuItem::create("Colors", "");
    color_buttons_menu->set_attribute_value(
        "custom", Glib::create_variant<Glib::ustring>("colors"));
    menu_model->insert_item(2, color_buttons_menu);

    // Setup Colors Radio
    Gtk::Box color_box{Gtk::Orientation::HORIZONTAL};
    color_box.set_spacing(2);
    color_box.set_margin(4);

    Gtk::CheckButton* prev = nullptr;

    for (const auto& [color_id, color] : CARD_COLORS) {
        auto checkbutton = Gtk::make_managed<Gtk::CheckButton>();

        checkbutton->set_tooltip_text(label_for_color(color_id));
        sigc::connection color_setting_cnn =
            checkbutton->signal_toggled().connect(
                color_setting_thunk(card_widget, checkbutton, color_id));
        if (color_id != CoverColor::UNSET) {
            checkbutton->add_css_class(cover_color_css(color_id));
            checkbutton->add_css_class("accent-color-btn");
        }
        color_box.append(*checkbutton);
        if (prev) checkbutton->set_group(*prev);
        prev = checkbutton;

        m_color_radio_button_map[color_id] =
            std::make_tuple(checkbutton, color_setting_cnn);
    }
    auto frame = Gtk::make_managed<Gtk::Frame>();
    frame->set_child(color_box);

    set_menu_model(menu_model);
    insert_action_group("card-popover", action_group);

    add_child(*frame, "colors");
}

CardWidget::CardPopover::~CardPopover() {
    std::erase(registered_card_popovers[m_card_widget], this);
    if (registered_card_popovers[m_card_widget].empty()) {
        registered_card_popovers.erase(m_card_widget);
    }

    spdlog::get("app")->debug("Registered Popovers: {}",
                              registered_card_popovers.size());
}

void CardWidget::CardPopover::set_selected_color(CoverColor color,
                                                 bool trigger) {
    auto color_checkbutton = std::get<0>(m_color_radio_button_map[color]);
    auto checkbutton_cnn = std::get<1>(m_color_radio_button_map[color]);
    if (!trigger) {
        checkbutton_cnn.block();
        color_checkbutton->set_active();
        checkbutton_cnn.unblock();
    } else {
        color_checkbutton->set_active();

        // Update all related popovers to this popover but do not trigger their
        // signal
        for (auto sibling : registered_card_popovers[m_card_widget]) {
            if (sibling != this) {
                sibling->set_selected_color(color, false);
            }
        }
    }
}

std::function<void()> CardWidget::CardPopover::color_setting_thunk(
    CardWidget* card, Gtk::CheckButton* checkbutton, CoverColor color) {
    return std::function<void()>([card, checkbutton, color]() {
        if (checkbutton->get_active()) {
            card->set_cover_color(color);
        }
    });
}

const std::array<std::string, 3> CardWidget::DATE_LABEL_CSS_CLASSES = {
    "due-date-complete",
    "past-due-date",
    "due-date",
};

const std::array<std::string, 3> CardWidget::TASKS_LABEL_CSS_CLASSES = {
    "complete-tasks-indicator-complete",
    "complete-tasks-indicator-almost",
    "complete-tasks-indicator-incomplete",
};

CardWidget::CardWidget(std::shared_ptr<Card> card)
    : Glib::ObjectBase{"CardWidget"},
      CardInit{},
      BaseItem{Gtk::Orientation::VERTICAL, 0},
      m_card{card},
      m_cardlist_widget{nullptr},
      m_root{Gtk::Orientation::VERTICAL},
      m_card_cover_revealer{},
      m_card_entry_revealer{},
      m_card_cover_picture{},
      m_card_label{},
      m_completion_label{},
      m_deadline_label{},
      m_card_entry{},
      m_card_menu_button{},
      m_fixed_card_popover{this},
      m_mouse_card_popover{this},
      m_key_ctrl{Gtk::EventControllerKey::create()},
      m_click_ctrl{Gtk::GestureClick::create()},
      m_focus_ctrl{Gtk::EventControllerFocus::create()} {
    setup_widgets();

    // CardWidget Setup
    set_title(card->get_name());

    m_focus_ctrl->signal_leave().connect(
        sigc::mem_fun(*this, &CardWidget::off_rename));
    m_card_entry.add_controller(m_focus_ctrl);

    m_card_cover_revealer.property_child_revealed().signal_changed().connect(
        [this]() {
            if (!m_card_cover_revealer.get_child_revealed())
                this->m_card_cover_picture.set_paintable(nullptr);
        });

    m_deadline_label.property_label().signal_changed().connect(
        sigc::mem_fun(*this, &CardWidget::update_deadline_label));

    using CardShortcut =
        std::pair<const char*,
                  std::function<bool(Gtk::Widget&, const Glib::VariantBase&)>>;

    const std::array<CardShortcut, 9> card_shortcuts = {
        {{"<Control>N",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              auto card_widget = m_cardlist_widget->insert_new_card_after(
                  Card{_("New Card")}, this);
              card_widget->grab_focus();
              return true;
          }},
         {"<Control>D",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              this->open_card_details_dialog();
              return true;
          }},
         {"<Control>R",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              this->on_rename();
              return true;
          }},
         {"<Control>Delete",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              this->remove_from_parent();
              return true;
          }},
         {"<Control>Up",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              CardWidget* previous_card =
                  static_cast<CardWidget*>(this->get_prev_sibling());
              if (previous_card) {
                  this->m_cardlist_widget->reorder(*previous_card, *this);
              } else {
                  this->error_bell();
              }
              return true;
          }},
         {"<Control>Down",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              Widget* next = this->get_next_sibling();
              if (!G_TYPE_CHECK_INSTANCE_TYPE(next->gobj(),
                                              Gtk::Button::get_type())) {
                  this->m_cardlist_widget->reorder(
                      *this, *static_cast<CardWidget*>(next));
              } else {
                  this->error_bell();
              }
              return true;
          }},
         {"<Control>Left",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              CardlistWidget* prev_parent = static_cast<CardlistWidget*>(
                  this->m_cardlist_widget->get_prev_sibling());
              if (prev_parent) {
                  this->remove_from_parent();
                  auto this_card = prev_parent->append_new_card(*this->m_card);
                  this_card->grab_focus();
              } else {
                  this->error_bell();
              }
              return true;
          }},
         {"<Control>Right",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              Widget* next_parent = this->m_cardlist_widget->get_next_sibling();
              if (!G_TYPE_CHECK_INSTANCE_TYPE(next_parent->gobj(),
                                              Gtk::Button::get_type())) {
                  CardlistWidget* next_cardlist =
                      static_cast<CardlistWidget*>(next_parent);
                  this->remove_from_parent();
                  auto this_card =
                      next_cardlist->append_new_card(*this->m_card);
                  this_card->grab_focus();
              } else {
                  this->error_bell();
              }
              return true;
          }},
         {"Menu|<Shift>F10", [this](Gtk::Widget& m, const Glib::VariantBase&) {
              this->m_mouse_card_popover.popup();
              return true;
          }}}};

    auto shortcut_controller = Gtk::ShortcutController::create();
    shortcut_controller->set_scope(Gtk::ShortcutScope::LOCAL);

    for (const auto& [keybinding, callback] : card_shortcuts) {
        shortcut_controller->add_shortcut(Gtk::Shortcut::create(
            Gtk::ShortcutTrigger::parse_string(keybinding),
            Gtk::CallbackAction::create(callback)));
    }
    this->add_controller(shortcut_controller);

    m_key_ctrl->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (m_card_entry_revealer.get_child_revealed()) {
                switch (keyval) {
                    case GDK_KEY_Return: {
                        this->on_confirm_changes();
                        this->off_rename();
                        break;
                    }
                    case GDK_KEY_Escape: {
                        this->off_rename();
                        break;
                    }
                }
            }
        });

    m_click_ctrl->set_button(0);
    m_click_ctrl->signal_released().connect(
        [this](int n_pressed, double x, double y) {
            auto clicked = this->m_click_ctrl->get_current_button();
            if (clicked == GDK_BUTTON_SECONDARY && n_pressed >= 1) {
                this->m_mouse_card_popover.set_pointing_to(
                    Gdk::Rectangle(x, y, 0, 0));
                this->m_mouse_card_popover.popup();
            } else if (n_pressed >= 1 &&
                       !m_card_entry_revealer.get_child_revealed() &&
                       clicked == GDK_BUTTON_PRIMARY) {
                this->on_rename();
            }
        });

    m_card_entry.add_controller(m_key_ctrl);
    add_controller(m_click_ctrl);

    if (card->is_color_set()) {
        Color color = card->get_color();
        CoverColor cover_color = rgb_to_cover_color(color);

        __set_cover_color(CARD_COLORS.at(cover_color));

        m_fixed_card_popover.set_selected_color(cover_color, false);
        m_mouse_card_popover.set_selected_color(cover_color, false);
    } else {
        m_fixed_card_popover.set_selected_color(CoverColor::UNSET, false);
        m_mouse_card_popover.set_selected_color(CoverColor::UNSET, false);
    }

    const Date card_date = card->get_due_date();
    if (card_date.ok()) {
        m_deadline_label.set_label(_("Due: ") + format_date_str(card_date));
        m_deadline_label.set_visible();
    }

    if (!card->container().get_data().empty()) {
        m_completion_label.set_visible();
        update_completion_label();
    }

    set_tooltip_markup(create_details_text());
    setup_drag_and_drop();
}

void CardWidget::set_title(const std::string& label) {
    m_card_label.set_label(label);
    m_card_entry.set_text(label);
}

void CardWidget::set_cardlist(CardlistWidget* new_parent) {
    if (new_parent) {
        this->m_cardlist_widget = new_parent;
    }
}

void CardWidget::set_cover_color(CoverColor color) {
    Gdk::RGBA rgb_color = CARD_COLORS.at(color);
    Color new_color =
        Color{rgb_color.get_red() * 255, rgb_color.get_green() * 255,
              rgb_color.get_blue() * 255, rgb_color.get_alpha()};
    m_card->set_color(new_color);

    if (color != CoverColor::UNSET) {
        __set_cover_color(rgb_color);

        spdlog::get("app")->info("Card (\"{}\") cover color set to (\"{}\")",
                                 m_card->get_name(),
                                 rgb_color.to_string().c_str());
    } else {
        __clear_cover_color();

        spdlog::get("app")->info("Card (\"{}\") cover color set to empty",
                                 m_card->get_name());
    }
}

void CardWidget::set_deadline(const Date& new_date) {
    Date old = m_card->get_due_date();

    m_card->set_due_date(new_date);
    if (new_date.ok()) {
        m_deadline_label.set_visible();
        m_deadline_label.set_label(_("Due: ") + format_date_str(new_date));

        if (old.ok()) {
            spdlog::get("app")->info(
                "Scheduled new due date for Card(\"{}\"): (\"{}\") -> "
                "(\"{}\")",
                m_card->get_name(), std::format("{}", old),
                std::format("{}", new_date));
        } else {
            spdlog::get("app")->info("Card(\"{}\") due date set to ({})",
                                     m_card->get_name(),
                                     std::format("{}", new_date));
        }
    } else {
        m_deadline_label.set_visible(false);
        spdlog::get("app")->info("Unset due date for Card (\"{}\")",
                                 m_card->get_name());
    }
}

void CardWidget::remove_from_parent() {
    if (m_cardlist_widget) {
        m_cardlist_widget->remove(*this);
    }
}

void CardWidget::update_deadline_label() {
    if (m_card->get_due_date().ok()) {
        std::string css_class;

        if (m_card->get_complete())
            css_class = "due-date-complete";
        else if (m_card->past_due_date())
            css_class = "past-due-date";
        else
            css_class = "due-date";

        for (const auto& css : DATE_LABEL_CSS_CLASSES) {
            if (m_deadline_label.has_css_class(css)) {
                if (css_class == css) return;  // Has already been set

                m_deadline_label.remove_css_class(css);
            }
        }
        m_deadline_label.add_css_class(css_class);
    }
}

void CardWidget::update_completion_label() {
    if (!m_card->container().get_data().empty()) {
        m_completion_label.set_visible(true);
        float n_complete_tasks =
            std::accumulate(m_card->container().get_data().begin(),
                            m_card->container().get_data().end(), 0,
                            [](int acc, const std::shared_ptr<Task>& task) {
                                return task->get_done() ? ++acc : acc;
                            });
        m_completion_label.set_label(std::format(
            "{}/{}", n_complete_tasks, m_card->container().get_data().size()));
        set_completion_label_color(n_complete_tasks);
    } else {
        m_completion_label.set_visible(false);
    }
}

std::string CardWidget::get_title() const { return m_card_label.get_label(); }

std::string CardWidget::create_details_text() const {
    using namespace std::chrono;

    std::ostringstream details_text;

    if (!m_card->container().get_data().empty()) {
        details_text << Glib::ustring::compose(
                            _("%1%% complete"),
                            Glib::ustring::format(std::fixed,
                                                  std::setprecision(0),
                                                  m_card->get_completion()))
                     << "\n\n";
    }

    auto card_due_date = m_card->get_due_date();
    if (card_due_date.ok()) {
        auto sys_clock_now = sys_days(floor<days>(system_clock::now()));
        Date cur_date = Date{sys_clock_now};
        if (m_card->get_complete()) {
            details_text << _("This card is complete") << "\n\n";
        } else if (card_due_date == cur_date) {
            details_text << _("The card is due today") << "\n\n";
        } else {
            details_text << time_info_text(cur_date, card_due_date);
        }
    }

    if (!m_card->get_notes().empty()) {
        details_text << "<b>" << _("Notes") << "</b>" << ": "
                     << truncate(m_card->get_notes()) << "\n\n";
    }

    std::string final_text = details_text.str();

    if (!final_text.empty()) {
        final_text.resize(final_text.size() - 2);
    }

    return final_text;
}

std::shared_ptr<Card> CardWidget::get_card() { return m_card; }

CardlistWidget const* CardWidget::get_cardlist_widget() const {
    return m_cardlist_widget;
}

void CardWidget::setup_drag_and_drop() {
    // DragSource Settings
    auto drag_source_c = Gtk::DragSource::create();
    drag_source_c->set_actions(Gdk::DragAction::MOVE);
    drag_source_c->signal_prepare().connect(
        [this, drag_source_c](double x, double y) {
            Glib::Value<CardWidget*> value_new_cardptr;
            value_new_cardptr.init(Glib::Value<CardWidget*>::value_type());
            value_new_cardptr.set(this);
            auto card_icon = Gtk::WidgetPaintable::create(*this);
            drag_source_c->set_icon(card_icon, x, y);
            return Gdk::ContentProvider::create(value_new_cardptr);
        },
        false);
    drag_source_c->signal_drag_begin().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref) {
            this->m_cardlist_widget->board.set_scroll();

            spdlog::get("ui")->debug("[CardWidget.dnd] (\"{}\"): On drag",
                                     m_card->get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref,
               Gdk::DragCancelReason reason) {
            this->m_cardlist_widget->board.set_scroll(false);

            spdlog::get("ui")->debug(
                "[CardWidget.dnd] (\"{}\"): Canceled drag event",
                m_card->get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref, bool s) {
            this->m_cardlist_widget->board.set_scroll(false);

            spdlog::get("ui")->debug(
                "[CardWidget.dnd] (\"{}\") Stopped being dragged",
                m_card->get_name());
        });
    add_controller(drag_source_c);

    // DropTarget Settings
    auto drop_target_c = Gtk::DropTarget::create(
        Glib::Value<CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_c->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->m_cardlist_widget->board.set_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<CardWidget*>::value_type())) {
                Glib::Value<CardWidget*> dropped_value;
                dropped_value.init(value.gobj());
                auto dropped_card_widget = dropped_value.get();

                if (dropped_card_widget == this) {
                    spdlog::warn(
                        "[CardWidget.dnd] (\"{}\") has been dropped on "
                        "itself",
                        m_card->get_name());

                    // After dropping, the receiver card still has the style
                    // set by DropControllerMotion. Reset
                    this->remove_css_class("card-to-drop");
                    return true;
                }

                spdlog::get("app")->info(
                    "Card (\"{}\") has been dropped on Card (\"{}\")",
                    dropped_card_widget->get_card()->get_name(),
                    m_card->get_name());

                if (dropped_card_widget->get_cardlist_widget() ==
                    this->m_cardlist_widget) {
                    this->m_cardlist_widget->reorder(*dropped_card_widget,
                                                     *this);
                } else {
                    auto dropped_card = dropped_card_widget->get_card();
                    CardWidget* dropped_copy =
                        this->m_cardlist_widget->append_new_card(*dropped_card);
                    dropped_card_widget->remove_from_parent();
                    this->m_cardlist_widget->reorder(*dropped_copy, *this);
                }

                // After dropping, the receiver card still has the style set
                // by DropControllerMotion. Reset
                this->remove_css_class("card-to-drop");

                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_c);
}

void CardWidget::open_card_details_dialog() {
    auto& parent_window = *(static_cast<ProgressWindow*>(get_root()));

    // This is a workaround for the issue where the Card widget maintains
    // its rename mode state on even when the user explicited entered card
    // details dialog
    off_rename();

    parent_window.show_card_dialog(this);
}

void CardWidget::on_rename() {
    // FIXME: Adding and removing the control focus fixes the bug where we
    // cannot rename a card through its menu options. Every time the user
    // (before this workaround) hit the rename button, the card would
    // quickly enter and leave rename mode, not allowing changes at all.
    m_card_entry.remove_controller(m_focus_ctrl);
    m_card_entry_revealer.set_reveal_child(true);
    m_card_label.set_visible(false);
    m_card_entry.grab_focus();
    m_card_entry.add_controller(m_focus_ctrl);

    spdlog::get("ui")->debug(
        "[CardWidget.on_rename] (\"{}\"): Entered rename mode",
        m_card->get_name());
}

void CardWidget::off_rename() {
    m_card_label.set_visible();
    m_card_entry_revealer.set_reveal_child(false);

    spdlog::get("ui")->debug(
        "[CardWidget.on_rename] (\"{}\"): Exited rename mode",
        m_card->get_name());
}

void CardWidget::on_confirm_changes() {
    if (m_card_entry.get_text().compare(m_card_label.get_label()) != 0) {
        const std::string old = m_card->get_name();

        m_card->set_name(m_card_entry.get_text());
        m_card_label.set_label(m_card_entry.get_text());

        spdlog::get("app")->info("Card (\"{}\") renamed to (\"{}\")", old,
                                 m_card->get_name());
    }
}

void CardWidget::set_completion_label_color(unsigned long n_complete_tasks) {
    std::string css_class;
    const float threshold = m_card->container().get_data().size() / 2.0F;

    if (n_complete_tasks == m_card->container().get_data().size()) {
        css_class = "complete-tasks-indicator-complete";
    } else if (n_complete_tasks < threshold) {
        css_class = "complete-tasks-indicator-incomplete";
    } else if (n_complete_tasks >= threshold) {
        css_class = "complete-tasks-indicator-almost";
    }

    if (m_completion_label.has_css_class(css_class)) {
        return;
    }

    for (const auto& css : TASKS_LABEL_CSS_CLASSES) {
        if (m_completion_label.has_css_class(css)) {
            m_completion_label.remove_css_class(css);
        }
    }

    m_completion_label.add_css_class(css_class);
}

void CardWidget::__set_cover_color(const Gdk::RGBA& color) {
    auto color_frame_pixbuf = Gdk::Pixbuf::create(
        Gdk::Colorspace::RGB, false, 8, CardlistWidget::CARDLIST_MAX_WIDTH, 30);
    color_frame_pixbuf->fill(rgb_to_hex(m_card->get_color()));

    m_card_cover_picture.set_paintable(
        Gdk::Texture::create_for_pixbuf(color_frame_pixbuf));
    m_card_cover_revealer.set_reveal_child(true);
}

void CardWidget::__clear_cover_color() {
    m_card_cover_revealer.set_reveal_child(false);
    m_card_cover_picture.set_paintable(nullptr);
}

void CardWidget::cleanup() { m_root.unparent(); }

void CardWidget::setup_widgets() {
    m_root.set_spacing(4);
    m_root.set_size_request(240, -1);

    // Card's cover
    m_root.append(m_card_cover_revealer);
    m_card_cover_revealer.set_child(m_card_cover_picture);
    m_card_cover_picture.set_content_fit(Gtk::ContentFit::COVER);
    m_card_cover_picture.add_css_class("card-cover");
    m_card_cover_picture.set_size_request(-1, 50);

    // Card Body
    Gtk::Box& card_body = *Gtk::make_managed<Gtk::Box>();
    card_body.set_margin(4);
    card_body.set_spacing(10);
    m_root.append(card_body);

    // inner box
    Gtk::Box& card_data_box =
        *Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    card_data_box.set_spacing(4);
    card_data_box.set_valign(Gtk::Align::CENTER);

    Gtk::Box& card_label_box =
        *Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    card_label_box.append(m_card_label);
    m_card_label.set_halign(Gtk::Align::START);
    m_card_label.set_hexpand();
    m_card_label.set_label(Glib::locale_to_utf8(_("New Card")));
    m_card_label.set_wrap();
    m_card_label.set_natural_wrap_mode(Gtk::NaturalWrapMode::WORD);
    m_card_label.set_wrap_mode(Pango::WrapMode::WORD_CHAR);

    card_label_box.append(m_card_entry_revealer);
    m_card_entry_revealer.set_child(m_card_entry);
    m_card_entry.set_hexpand();
    card_data_box.append(card_label_box);

    Gtk::Box& card_info_box = *Gtk::make_managed<Gtk::Box>();
    card_info_box.set_spacing(15);

    card_info_box.append(m_completion_label);
    m_completion_label.set_visible(false);

    card_info_box.append(m_deadline_label);
    m_deadline_label.set_visible(false);

    card_data_box.append(card_info_box);

    card_body.append(card_data_box);

    m_card_menu_button.set_halign(Gtk::Align::CENTER);
    m_card_menu_button.set_has_frame(false);
    m_card_menu_button.set_icon_name("view-more-horizontal-symbolic");
    m_card_menu_button.set_can_focus(false);
    m_card_menu_button.set_popover(m_fixed_card_popover);

    m_mouse_card_popover.set_parent(m_root);

    m_card_menu_button.set_tooltip_text(_("Card Options"));
    m_card_menu_button.set_valign(Gtk::Align::CENTER);
    card_body.append(m_card_menu_button);

    m_root.insert_at_end(*this);
}
}  // namespace ui
