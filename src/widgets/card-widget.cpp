#include "card-widget.h"

#include <dialog/card-dialog.h>
#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <window.h>

#include <numeric>

#include "cardlist-widget.h"

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

using ButtonAction =
    std::tuple<const char*, const char*, std::function<void()>>;

namespace ui {
CardInit::CardInit()
    : Glib::ExtraClassInit(card_class_init, nullptr, card_init) {}

const std::map<const char*, std::pair<const char*, const char*>>
    CardWidget::CardPopover::CARD_COLORS = {
        {"red", {_("Red"), "#a51d2d"}},
        {"orange", {_("Orange"), "#c64600"}},
        {"yellow", {_("Yellow"), "#e5a50a"}},
        {"green", {_("Green"), "#26a269"}},
        {"blue", {_("Blue"), "#1a5fb4"}},
        {"purple", {_("Purple"), "#200941"}}};

std::map<CardWidget*, std::vector<CardWidget::CardPopover*>>
    CardWidget::CardPopover::card_popovers{};

void CardWidget::CardPopover::mass_color_select(CardWidget* key,
                                                Gdk::RGBA color) {
    const std::vector<CardPopover*>& popovers = card_popovers[key];
    for (const auto& popover : popovers) {
        popover->set_selected_color(color, false);
    }

    spdlog::get("ui")->debug("[CardPopover] Set color to {} for all popovers",
                             color.to_string().c_str());
}

CardWidget::CardPopover::CardPopover(CardWidget* card_widget)
    : Gtk::Popover{},
      card_widget{card_widget},
      root{Gtk::Orientation::VERTICAL} {
    if (!card_popovers.contains(card_widget)) {
        card_popovers[card_widget] = std::vector<CardPopover*>{this};
    } else {
        card_popovers[card_widget].push_back(this);
    }

    set_has_arrow(false);
    set_child(root);
    set_position(Gtk::PositionType::BOTTOM);

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
                         card_widget->remove_from_parent();
                         this->popdown();
                     }}};

    for (const auto& [key, label, action] : button_actions) {
        auto button = Gtk::make_managed<Gtk::Button>(label);
        button->set_has_frame(false);
        button->signal_clicked().connect(action);
        root.append(*button);

        action_buttons[key] = button;
    }

    // Setup Colors Radio
    Gtk::Box color_box{Gtk::Orientation::HORIZONTAL};

    Gtk::CheckButton* prev = Gtk::make_managed<Gtk::CheckButton>();
    prev->set_tooltip_text(_("Unset Color"));
    sigc::connection unset_color_cnn =
        prev->signal_toggled().connect([prev, card_widget]() {
            if (prev->get_active()) {
                card_widget->get_card()->set_color(NO_COLOR);
            }
        });
    color_box.append(*prev);

    // In this current context, no colour whatsoever means an invisible color.
    // This helps the popover check if a colour has been set
    color_buttons["Unset"] =
        std::make_tuple(prev, "rgba(0,0,0,0)", unset_color_cnn);

    for (const auto& [key, color_pair] : CARD_COLORS) {
        auto checkbutton = Gtk::make_managed<Gtk::CheckButton>();

        checkbutton->set_tooltip_text(_(color_pair.first));
        sigc::connection color_setting_cnn =
            checkbutton->signal_toggled().connect(color_setting_thunk(
                card_widget, checkbutton, Gdk::RGBA{color_pair.second}));
        checkbutton->add_css_class(key);
        checkbutton->add_css_class("accent-color-btn");
        color_box.append(*checkbutton);
        checkbutton->set_group(*prev);
        prev = checkbutton;

        color_buttons[color_pair.first] =
            std::make_tuple(checkbutton, color_pair.second, color_setting_cnn);
    }
    auto frame = Gtk::make_managed<Gtk::Frame>();
    frame->set_child(color_box);
    root.insert_child_after(*frame, *action_buttons["card-details"]);
}

void CardWidget::CardPopover::set_selected_color(Gdk::RGBA color,
                                                 bool trigger) {
    if (!trigger) disable_color_signals();
    for (auto& [label, colour_tuple] : color_buttons) {
        if (Gdk::RGBA(std::get<1>(colour_tuple)) == color) {
            // Select the Checkbutton pointer refering to this colour and set it
            // as active
            std::get<0>(colour_tuple)->set_active(true);
            if (!trigger) enable_color_signals();
            return;
        }
    }
    if (!trigger) enable_color_signals();
    std::get<0>(color_buttons["Unset"])->set_active(true);
}

void CardWidget::CardPopover::disable_color_signals() {
    for (auto& [color_label, data] : color_buttons) {
        std::get<2>(data).block();
    }

    spdlog::get("ui")->debug("[CardPopover] Color signals disabled");
}

void CardWidget::CardPopover::enable_color_signals() {
    for (auto& [color_label, data] : color_buttons) {
        std::get<2>(data).unblock();
    }

    spdlog::get("ui")->debug("[CardPopover] Color signals enabled");
}

std::function<void()> CardWidget::CardPopover::color_setting_thunk(
    CardWidget* card, Gtk::CheckButton* checkbutton, Gdk::RGBA color) {
    return std::function<void()>([card, checkbutton, color]() {
        if (checkbutton->get_active()) {
            card->set_cover_color(color);
            spdlog::get("ui")->debug("[CardPopover] Set color to {}",
                                     color.to_string().c_str());
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

inline std::string format_date_str(Date date) {
    const std::time_t time =
        std::chrono::system_clock::to_time_t(std::chrono::sys_days(date));

    char date_str[30];
    std::strftime(date_str, sizeof(date_str), "%d %b, %Y", std::gmtime(&time));

    return date_str;
}

CardWidget::CardWidget(std::shared_ptr<Card> card, bool is_new)
    : Glib::ObjectBase{"CardWidget"},
      CardInit{},
      BaseItem{Gtk::Orientation::VERTICAL, 0},
      card{card},
      parent{nullptr},
      is_new{is_new},
      root_box{Gtk::Orientation::VERTICAL},
      card_cover_revealer{},
      card_entry_revealer{},
      card_cover_picture{},
      card_label{},
      complete_tasks_label{},
      due_date_label{},
      card_entry{},
      card_menu_button{},
      card_menu_popover{this},
      card_menu_popover2{this},
      key_controller{Gtk::EventControllerKey::create()},
      click_controller{Gtk::GestureClick::create()},
      focus_controller{Gtk::EventControllerFocus::create()},
      color_dialog{Gtk::ColorDialog::create()} {
    // Setup Widgets
    root_box.set_spacing(4);
    root_box.set_size_request(240, -1);

    // Card's cover
    root_box.append(card_cover_revealer);
    card_cover_revealer.set_child(card_cover_picture);
    card_cover_picture.set_content_fit(Gtk::ContentFit::COVER);
    card_cover_picture.add_css_class("card-cover");
    card_cover_picture.set_size_request(-1, 50);

    // Card Body
    Gtk::Box& card_body = *Gtk::make_managed<Gtk::Box>();
    card_body.set_margin(4);
    card_body.set_spacing(10);
    root_box.append(card_body);

    // inner box
    Gtk::Box& card_data_box =
        *Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    card_data_box.set_spacing(4);
    card_data_box.set_valign(Gtk::Align::CENTER);

    Gtk::Box& card_label_box =
        *Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    card_label_box.append(card_label);
    card_label.set_halign(Gtk::Align::START);
    card_label.set_hexpand();
    card_label.set_label(Glib::locale_to_utf8(_("New Card")));
    card_label.set_wrap();
    card_label.set_natural_wrap_mode(Gtk::NaturalWrapMode::WORD);
    card_label.set_wrap_mode(Pango::WrapMode::WORD_CHAR);

    card_label_box.append(card_entry_revealer);
    card_entry_revealer.set_child(card_entry);
    card_entry.set_hexpand();
    focus_controller->signal_leave().connect(
        sigc::mem_fun(*this, &CardWidget::off_rename));
    card_entry.add_controller(focus_controller);
    card_data_box.append(card_label_box);

    Gtk::Box& card_info_box = *Gtk::make_managed<Gtk::Box>();
    card_info_box.set_spacing(15);

    card_info_box.append(complete_tasks_label);
    complete_tasks_label.set_visible(false);

    card_info_box.append(due_date_label);
    due_date_label.add_css_class("due-date");
    due_date_label.set_visible(false);

    card_data_box.append(card_info_box);

    card_body.append(card_data_box);

    card_menu_button.set_halign(Gtk::Align::CENTER);
    card_menu_button.set_has_frame(false);
    card_menu_button.set_icon_name("view-more-horizontal-symbolic");
    card_menu_button.set_can_focus(false);
    card_menu_button.set_popover(card_menu_popover);

    card_menu_popover2.set_parent(root_box);

    card_menu_button.set_tooltip_text(_("Card Options"));
    card_menu_button.set_valign(Gtk::Align::CENTER);
    card_body.append(card_menu_button);

    root_box.insert_at_end(*this);

    // CardWidget Setup

    set_title(card->get_name());

    if (is_new) {
        on_rename();  // Open card on rename mode by default whenever a new card
                      // is created
    }

    card_cover_revealer.property_child_revealed().signal_changed().connect(
        [this]() {
            if (!card_cover_revealer.get_child_revealed())
                this->card_cover_picture.set_paintable(nullptr);
        });

    due_date_label.property_label().signal_changed().connect(
        sigc::mem_fun(*this, &CardWidget::update_due_date_label));

    card->signal_color().connect(
        sigc::mem_fun(*this, &CardWidget::color_handler));
    card->signal_due_date().connect(
        sigc::mem_fun(*this, &CardWidget::due_date_handler));
    card->signal_complete().connect(
        sigc::track_object([this](bool) { update_due_date_label(); }, *this));
    card->container().signal_append().connect(
        sigc::mem_fun(*this, &CardWidget::on_append_handle));
    card->container().signal_remove().connect(
        sigc::mem_fun(*this, &CardWidget::on_remove_handle));

    using CardShortcut =
        std::pair<const char*,
                  std::function<bool(Gtk::Widget&, const Glib::VariantBase&)>>;

    const std::array<CardShortcut, 9> card_shortcuts = {
        {{"<Control>N",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              auto new_card = parent->add(Card{_("New Card")}, true);
              parent->reorder(*new_card, *this);
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
         {"<Control><Shift>C",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              this->open_color_dialog();
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
                  this->parent->reorder(*previous_card, *this);
              }
              return true;
          }},
         {"<Control>Down",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              Widget* next = this->get_next_sibling();
              if (!G_TYPE_CHECK_INSTANCE_TYPE(next->gobj(),
                                              Gtk::Button::get_type())) {
                  this->parent->reorder(*this, *static_cast<CardWidget*>(next));
              }
              return true;
          }},
         {"<Control>Left",
          [this](Gtk::Widget&, const Glib::VariantBase&) {
              CardlistWidget* prev_parent = static_cast<CardlistWidget*>(
                  this->parent->get_prev_sibling());
              if (prev_parent) {
                  this->remove_from_parent();
                  auto this_card = prev_parent->add(*this->card);
                  this_card->grab_focus();
              }
              return true;
          }},
         {"<Control>Right", [this](Gtk::Widget&, const Glib::VariantBase&) {
              Widget* next_parent = this->parent->get_next_sibling();
              if (!G_TYPE_CHECK_INSTANCE_TYPE(next_parent->gobj(),
                                              Gtk::Button::get_type())) {
                  CardlistWidget* next_cardlist =
                      static_cast<CardlistWidget*>(next_parent);
                  this->remove_from_parent();
                  auto this_card = next_cardlist->add(*this->card);
                  this_card->grab_focus();
              }
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

    key_controller->signal_key_released().connect(
        [this](guint keyval, guint keycode, Gdk::ModifierType state) {
            if (card_entry_revealer.get_child_revealed()) {
                switch (keyval) {
                    case GDK_KEY_Return: {
                        this->on_confirm_changes();
                        this->off_rename();
                        break;
                    }
                    case GDK_KEY_Escape: {
                        this->on_cancel_changes();
                        this->off_rename();
                        break;
                    }
                }
            }
        });

    click_controller->set_button(0);
    click_controller->signal_released().connect(
        [this](int n_pressed, double x, double y) {
            auto clicked = this->click_controller->get_current_button();
            if (clicked == GDK_BUTTON_SECONDARY && n_pressed >= 1) {
                this->card_menu_popover2.set_pointing_to(
                    Gdk::Rectangle(x, y, 0, 0));
                this->card_menu_popover2.popup();
            } else if (n_pressed >= 1 &&
                       !card_entry_revealer.get_child_revealed() &&
                       clicked == GDK_BUTTON_PRIMARY) {
                this->on_rename();
            }
        });

    auto drop_motion_controller = Gtk::DropControllerMotion::create();
    // drop_motion_controller->signal_motion().connect(
    //     [this](double x, double y) { this->add_css_class("card-to-drop"); });
    // drop_motion_controller->signal_leave().connect(
    //     [this]() { this->remove_css_class("card-to-drop"); });

    card_entry.add_controller(key_controller);
    add_controller(click_controller);
    add_controller(drop_motion_controller);

    if (card->is_color_set()) {
        Color card_color = card->get_color();
        Gdk::RGBA card_color_rgba = Gdk::RGBA{
            std::get<0>(card_color) / 255.0F, std::get<1>(card_color) / 255.0F,
            std::get<2>(card_color) / 255.0F, std::get<3>(card_color)};

        __set_cover_color(card_color_rgba);

        card_menu_popover.set_selected_color(card_color_rgba, false);
        card_menu_popover2.set_selected_color(card_color_rgba, false);
    } else {
        card_menu_popover.set_selected_color(Gdk::RGBA{}, false);
        card_menu_popover2.set_selected_color(Gdk::RGBA{}, false);
    }

    const Date card_date = card->get_due_date();
    if (card_date.ok()) {
        due_date_label.set_label(_("Due: ") + format_date_str(card_date));
        due_date_label.set_visible();
    }

    if (!card->container().get_data().empty()) {
        complete_tasks_label.set_visible();
        update_complete_tasks_label();
    }

    set_tooltip_markup(create_details_text());
    setup_drag_and_drop();
}

void CardWidget::set_title(const std::string& label) {
    card_label.set_label(label);
    card_entry.set_text(label);
}

void CardWidget::set_cardlist(CardlistWidget* new_parent) {
    if (new_parent) {
        if (this->parent) {
            spdlog::get("ui")->debug(
                "[CardWidget] CardWidget \"{}\" changed parents: "
                "(CardlistWidget \"{}\") -> (CardlistWidget \"{}\")",
                card->get_name(), this->parent->cardlist()->get_name(),
                new_parent->cardlist()->get_name());
        }
        this->parent = new_parent;
    }
}

void CardWidget::set_cover_color(const Gdk::RGBA& color) {
    card->set_color(Color{color.get_red() * 255, color.get_green() * 255,
                          color.get_blue() * 255, 1.0});

    spdlog::get("ui")->debug("[CardWidget] CardWidget \"{}\" has set color: {}",
                             card->get_name(), color.to_string().c_str());
}

void CardWidget::remove_from_parent() {
    if (parent) {
        parent->remove(*this);
    }
}

void CardWidget::update_due_date_label() {
    if (card->get_due_date().ok()) {
        std::string css_class;

        if (card->get_complete())
            css_class = "due-date-complete";
        else if (card->past_due_date())
            css_class = "past-due-date";
        else
            css_class = "due-date";

        for (const auto& css : DATE_LABEL_CSS_CLASSES) {
            if (css == css_class)
                due_date_label.add_css_class(css_class);
            else if (due_date_label.has_css_class(css))
                due_date_label.remove_css_class(css);
        }
    }
}

void CardWidget::update_complete_tasks_label() {
    float n_complete_tasks =
        std::accumulate(card->container().get_data().begin(),
                        card->container().get_data().end(), 0,
                        [](int acc, const std::shared_ptr<Task>& task) {
                            return task->get_done() ? ++acc : acc;
                        });
    complete_tasks_label.set_label(std::format(
        "{}/{}", n_complete_tasks, card->container().get_data().size()));
    update_complete_tasks_style(n_complete_tasks);
}

std::string CardWidget::get_title() const { return card_label.get_label(); }

std::string CardWidget::create_details_text() const {
    using namespace std::chrono;

    std::ostringstream details_text;

    if (!card->container().get_data().empty()) {
        details_text << Glib::ustring::compose(
                            _("%1%% complete"),
                            Glib::ustring::format(std::fixed,
                                                  std::setprecision(2),
                                                  card->get_completion()))
                     << "\n\n";
    }

    auto card_due_date = card->get_due_date();
    if (card_due_date.ok()) {
        auto sys_clock_now = sys_days(floor<days>(system_clock::now()));
        Date cur_date = Date{sys_clock_now};
        if (card->get_complete()) {
            details_text << _("This card is complete") << "\n\n";
        } else if (card_due_date == cur_date) {
            details_text << _("The card is due today") << "\n\n";
        } else if (!card->past_due_date()) {
            auto now = sys_days(cur_date);
            auto days = sys_days(card_due_date);
            auto delta_days = (days - now).count();

            if (delta_days < 30) {
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext(
                               "<b>%1 day</b> remaining",
                               "<b>%1 days</b> remaining", delta_days)),
                           delta_days)
                    << "\n\n";
            } else if (delta_days >= 30 && delta_days < 365) {
                long months_from_delta =
                    delta_days / 30;  // Assume every month has 30 days
                details_text
                    << Glib::ustring::compose(Glib::locale_to_utf8(ngettext(
                                                  "<b>%1 month</b> remaining",
                                                  "<b>%1 months</b> remaining",
                                                  months_from_delta)),
                                              months_from_delta)
                    << "\n\n";
            } else if (delta_days >= 365) {
                long years_from_delta = delta_days / 365;  // Ignore leap years
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext(
                               "<b>%1 year</b> remaining",
                               "<b>%1 years</b> remaining", years_from_delta)),
                           years_from_delta)
                    << "\n\n";
            }
        } else {
            auto now = sys_days(cur_date);
            auto days = sys_days(card_due_date);
            auto delta_days = -(days - now).count();

            if (delta_days < 30) {
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext(
                               "This card is past due date <b>%1 day ago</b>",
                               "This card is past due date <b>%1 days ago</b>",
                               delta_days)),
                           delta_days)
                    << "\n\n";
            } else if (delta_days >= 30 && delta_days < 365) {
                long months_from_delta =
                    delta_days / 30;  // Assume every month has 30 days
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext(
                               "This card is past due date <b>%1 month</b> ago",
                               "This card is past due date <b>%1 months</b> "
                               "ago",
                               months_from_delta)),
                           months_from_delta)
                    << "\n\n";
            } else if (delta_days >= 365) {
                long years_from_delta = delta_days / 365;  // Ignore leap years
                details_text
                    << Glib::ustring::compose(
                           Glib::locale_to_utf8(ngettext(
                               "This card is past due date <b>%1 year</b> ago",
                               "This card is past due date <b>%1 years</b> ago",
                               years_from_delta)),
                           years_from_delta)
                    << "\n\n";
            }
        }
    }

    if (!card->container().get_data().empty()) {
        details_text << "<b>" << _("Checklist") << "</b>" << ":\n";
        int c = 0;
        const int max_visible = 5;
        for (const auto& task : card->container()) {
            if (c == max_visible) {
                details_text << "\n...";
                break;
            }

            details_text << (task->get_done() ? "- [X] " : "- [ ] ")
                         << task->get_name();
            if (c < max_visible - 1 &&
                task != card->container().get_data().back()) {
                details_text << "\n";
            }
            c++;
        }
        details_text << "\n\n";
    }

    if (!card->get_notes().empty()) {
        details_text << "<b>" << _("Notes") << "</b>" << ":\n"
                     << Glib::ustring{card->get_notes()}.truncate_middle(150);
    }

    std::string final_text = details_text.str();

    if (!final_text.empty()) {
        final_text.resize(final_text.size() - 2);
    }

    return final_text;
}

std::shared_ptr<Card> CardWidget::get_card() { return card; }

CardlistWidget const* CardWidget::get_cardlist_widget() const { return parent; }

void CardWidget::on_append_handle(std::shared_ptr<Task> task) {
    complete_tasks_label.set_visible();
    update_complete_tasks_label();
}

void CardWidget::on_remove_handle(std::shared_ptr<Task> task) {
    if (card->container().get_data().empty()) {
        complete_tasks_label.set_visible(false);
    } else {
        update_complete_tasks_label();
    }
}

void CardWidget::due_date_handler(Date old, Date new_) {
    if (new_.ok()) {
        due_date_label.set_visible();

        due_date_label.set_label(_("Due: ") + format_date_str(new_));
    } else {
        due_date_label.set_visible(false);
    }

    spdlog::get("ui")->debug(std::format(
        "[CardWidget] CardWidget \"{}\"'s due date label update: {} -> {}",
        card->get_name(), old, new_));
}

void CardWidget::color_handler(Color old, Color new_) {
    if (new_ != NO_COLOR) {
        Gdk::RGBA c(float(std::get<0>(new_)) / 255,
                    float(std::get<1>(new_)) / 255,
                    float(std::get<2>(new_)) / 255, float(std::get<3>(new_)));
        __set_cover_color(c);
        CardWidget::CardPopover::mass_color_select(this, c);
    } else {
        __clear_cover_color();
    }
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
            this->parent->board.set_scroll();

            spdlog::get("ui")->debug(
                "[CardWidget] CardWidget \"{}\" is being dragged",
                card->get_name());
        },
        false);
    drag_source_c->signal_drag_cancel().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref,
               Gdk::DragCancelReason reason) {
            this->parent->board.set_scroll(false);

            spdlog::get("ui")->debug(
                "[CardWidget] CardWidget \"{}\" dragging has been canceled",
                card->get_name());
            return true;
        },
        false);
    drag_source_c->signal_drag_end().connect(
        [this](const Glib::RefPtr<Gdk::Drag>& drag_ref, bool s) {
            this->parent->board.set_scroll(false);

            spdlog::get("ui")->debug(
                "[CardWidget] CardWidget \"{}\" stopped being dragged",
                card->get_name());
        });
    add_controller(drag_source_c);

    // DropTarget Settings
    auto drop_target_c = Gtk::DropTarget::create(
        Glib::Value<CardWidget*>::value_type(), Gdk::DragAction::MOVE);
    drop_target_c->signal_drop().connect(
        [this](const Glib::ValueBase& value, double x, double y) {
            this->parent->board.set_scroll(false);
            if (G_VALUE_HOLDS(value.gobj(),
                              Glib::Value<CardWidget*>::value_type())) {
                Glib::Value<CardWidget*> dropped_value;
                dropped_value.init(value.gobj());
                auto dropped_card = dropped_value.get();

                if (dropped_card == this) {
                    spdlog::warn(
                        "[CardWidget] CardWidget \"{}\" has been dropped to "
                        "itself",
                        card->get_name());

                    // After dropping, the receiver card still has the style set
                    // by DropControllerMotion. Reset
                    this->remove_css_class("card-to-drop");
                    return true;
                }

                if (this->parent->is_child(*dropped_card)) {
                    this->parent->reorder(*dropped_card, *this);
                } else {
                    auto card_from_dropped = dropped_card->get_card();
                    CardWidget* dropped_copy =
                        this->parent->add(*card_from_dropped);
                    dropped_card->remove_from_parent();
                    this->parent->reorder(*dropped_copy, *this);
                }

                // After dropping, the receiver card still has the style set by
                // DropControllerMotion. Reset
                this->remove_css_class("card-to-drop");

                return true;
            }
            return false;
        },
        false);
    add_controller(drop_target_c);
}

void CardWidget::open_color_dialog() {
    color_dialog->choose_rgba(
        *static_cast<Gtk::Window*>(get_root()),
        [this](const Glib::RefPtr<Gio::AsyncResult>& result) {
            try {
                set_cover_color(color_dialog->choose_rgba_finish(result));
            } catch (Gtk::DialogError& err) {
                spdlog::get("ui")->warn(
                    "[CardWidget] CardWidget \"{}\" has failed to set color: "
                    "{}",
                    card->get_name(), err.what());
            }
        });

    spdlog::get("ui")->debug(
        "[CardWidget] CardWidget \"{}\" has opened color dialog",
        card->get_name());
}

void CardWidget::open_card_details_dialog() {
    auto& parent_window = *(static_cast<ProgressWindow*>(get_root()));
    parent_window.show_card_dialog(this);
}

void CardWidget::on_rename() {
    card_entry.remove_controller(focus_controller);
    card_entry_revealer.set_reveal_child(true);
    card_label.set_visible(false);
    card_entry.grab_focus();
    card_entry.add_controller(focus_controller);

    spdlog::get("ui")->debug(
        "[CardWidget] CardWidget \"{}\" has entered rename mode",
        card->get_name());
}

void CardWidget::off_rename() {
    card_label.set_visible();
    card_entry_revealer.set_reveal_child(false);

    spdlog::get("ui")->debug(
        "[CardWidget] CardWidget \"{}\" has exited rename mode",
        card->get_name());
}

void CardWidget::on_confirm_changes() {
    if (card_entry.get_text().compare(card_label.get_label()) != 0) {
        card->set_name(card_entry.get_text());
        card_label.set_label(card_entry.get_text());
    }
    is_new = false;
}

void CardWidget::on_cancel_changes() {
    if (is_new) {
        remove_from_parent();
    }
}

void CardWidget::update_complete_tasks_style(unsigned long n_complete_tasks) {
    std::string css_class;
    const float threshold = card->container().get_data().size() / 2.0F;

    if (n_complete_tasks == card->container().get_data().size()) {
        css_class = "complete-tasks-indicator-complete";
    } else if (n_complete_tasks < threshold) {
        css_class = "complete-tasks-indicator-incomplete";
    } else if (n_complete_tasks >= threshold) {
        css_class = "complete-tasks-indicator-almost";
    }

    for (const auto& css : TASKS_LABEL_CSS_CLASSES) {
        if (css == css_class) {
            complete_tasks_label.add_css_class(css_class);
        } else if (complete_tasks_label.has_css_class(css)) {
            complete_tasks_label.remove_css_class(css);
        }
    }
}

void CardWidget::__set_cover_color(const Gdk::RGBA& color) {
    auto color_frame_pixbuf = Gdk::Pixbuf::create(
        Gdk::Colorspace::RGB, false, 8, CardlistWidget::CARDLIST_MAX_WIDTH, 30);
    color_frame_pixbuf->fill(rgb_to_hex(card->get_color()));

    if (card_cover_picture.get_paintable()) {
        card_cover_picture.set_paintable(nullptr);
    }

    card_cover_picture.set_paintable(
        Gdk::Texture::create_for_pixbuf(color_frame_pixbuf));
    card_cover_revealer.set_reveal_child(true);
}

void CardWidget::__clear_cover_color() {
    card_cover_revealer.set_reveal_child(false);

    CardWidget::CardPopover::mass_color_select(this, Gdk::RGBA{});

    spdlog::get("ui")->debug("[CardWidget] CardWidget \"{}\" has cleared color",
                             card->get_name());
}

void CardWidget::cleanup() { root_box.unparent(); }
}  // namespace ui
