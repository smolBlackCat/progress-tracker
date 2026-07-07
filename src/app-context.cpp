#include "app-context.h"

#include <glibmm/i18n.h>
#include <spdlog/spdlog.h>
#include <window.h>

#include <numeric>

#include "core/board.h"
#include "core/colorable.h"
#include "glibmm/main.h"
#include "widgets/card-widget.h"
#include "widgets/cardlist-widget.h"
#include "widgets/task-widget.h"

ui::CardWidget* AppContext::builder_card_widget(
    const std::shared_ptr<Card>& card) {
    Gdk::RGBA card_color =
        Gdk::RGBA{static_cast<float>(std::get<0>(card->get_color()) / 255.0),
                  static_cast<float>(std::get<1>(card->get_color()) / 255.0),
                  static_cast<float>(std::get<2>(card->get_color()) / 255.0),
                  std::get<3>(card->get_color())};

    Glib::Date card_deadline{};
    if (card->get_due_date().ok()) {
        card_deadline.set_dmy(unsigned(card->get_due_date().day()),
                              static_cast<Glib::Date::Month>(
                                  unsigned(card->get_due_date().month())),
                              int(card->get_due_date().year()));
    }
    int n_complete_tasks =
        std::accumulate(card->container().begin(), card->container().end(), 0,
                        [](int acc, const std::shared_ptr<Task> task) {
                            return task->get_done() ? ++acc : acc;
                        });

    ui::CardWidget* card_widget = Gtk::make_managed<ui::CardWidget>(
        card->get_name(), card_color, card_deadline, card->get_complete(),
        !card->get_notes().empty(), card->container().size(), n_complete_tasks);

    return card_widget;
}

AppContext::AppContext(ui::ProgressWindow& app_window,
                       ui::BoardWidget& board_widget, BoardManager& manager)
    : m_app_window(app_window),
      m_board_widget(board_widget),
      m_manager(manager) {
    m_app_window.signal_close_request().connect(
        sigc::mem_fun(*this, &AppContext::on_window_closed), true);
    m_load_board_dispatcher.connect(
        sigc::mem_fun(*this, &AppContext::on_session_loaded));
    m_save_board_dispatcher.connect(
        sigc::mem_fun(*this, &AppContext::on_session_saved));

    m_manager.signal_add_board().connect([this](const LocalBoard& local_board) {
        spdlog::get("app")->info("User has created board \"{}\"",
                                 local_board.board->get_name());
    });
    m_manager.signal_remove_board().connect(
        [this](const LocalBoard& local_board) {
            spdlog::get("app")->info("User has deleted board \"{}\"",
                                     local_board.board->get_name());
        });
}

void AppContext::open_session(const std::string& filename) {
    spdlog::get("app")->debug(
        "[AppContext.open_session] Dispatch board session starter thread");
    m_board_load_thread = std::thread{[this, filename]() {
        try {
            this->m_current_board = this->m_manager.local_open(filename);
        } catch (std::invalid_argument& err) {
            this->m_current_board = nullptr;
        }

        m_load_board_dispatcher.emit();
    }};
}

void AppContext::close_session() {
    if (m_current_board) {
        if (m_board_save_thread.joinable()) {
            spdlog::get("app")->debug(
                "[AppContext.close_session] Saver worker thread still running. "
                "Joining");
            m_board_save_thread.join();
        } else {
            m_manager.local_save(m_current_board);
        }
        m_manager.local_close(m_current_board);

        spdlog::get("app")->info("(\"{}\") → Closed",
                                 m_current_board->get_name());

        reset_session_state();

        // Any pending loading operations will be immediatelly cancelled
        m_session_flags[Status::LOADING] = false;
        if (!m_board_widget.empty()) {
            m_session_flags[Status::CLEARING] = true;
            Glib::signal_idle().connect(
                sigc::mem_fun(*this, &AppContext::idle_clear_session),
                Glib::PRIORITY_LOW);
        }
    }
}

void AppContext::reset_session_state() {
    // Reset card updating system state
    m_next_card_i = 0;
    m_cardlist_i = 0;
    m_cards.clear();

    // Reset idle, suspend, timeout event handlers
    m_idle_load_session_cnn.disconnect();
    m_timeout_save_cnn.disconnect();
    m_timeout_cards_update_cnn.disconnect();
#if GTKMM_CHECK_VERSION(4, 12, 0)
    m_suspended_tracker_cnn.disconnect();
#else
    g_signal_handler_disconnect(m_app_window.gobj(),
                                m_suspended_tracker_handler_id);
#endif

    clear_binds();

    m_current_board = nullptr;
}

void AppContext::on_session_loaded() {
    if (m_board_load_thread.joinable()) {
        m_board_load_thread.join();
    }

    if (m_current_board) {
        spdlog::get("app")->info("(\"{}\") → Started",
                                 m_current_board->get_name());
        m_session_flags[Status::LOADING] = true;

        m_app_window.set_title(m_current_board->get_name());
        ui::CardDialog& card_dialog = m_app_window.card_dialog();

        // TODO: Extract this into a AppContext::bind overload
        m_card_dialog_cnns.push_back(card_dialog.signal_open().connect(
            [this, &card_dialog](ui::CardWidget* card) {
                spdlog::get("app")->info("(\"{}\") → Card Dialog opened for {}",
                                         m_current_board->get_name(),
                                         card->get_title());

                auto db_card = m_bound_cards[card];

                card_dialog.set_title(db_card->get_name());

                Date db_card_date = db_card->get_due_date();
                if (db_card_date.ok()) {
                    Glib::Date deadline_date{};
                    deadline_date.set_dmy(unsigned(db_card_date.day()),
                                          static_cast<Glib::Date::Month>(
                                              unsigned(db_card_date.month())),
                                          int(db_card_date.year()));
                    card_dialog.set_deadline(deadline_date);
                    card_dialog.set_complete(db_card->get_complete());
                }

                for (auto& task : db_card->container()) {
                    ui::TaskWidget* task_widget =
                        Gtk::make_managed<ui::TaskWidget>(
                            card_dialog, task->get_name(), task->get_done());
                    bind(task, task_widget);
                    card_dialog.append(*task_widget);
                }

                card_dialog.set_notes(db_card->get_notes());

                m_card_dialog_cnns.push_back(
                    card_dialog.signal_task_added().connect(
                        [this, &card_dialog](ui::TaskWidget* task_w,
                                             int index) {
                            auto db_card =
                                m_bound_cards[card_dialog.card_widget()];

                            if (index == -1) {
                                auto new_db_task =
                                    Task::create(task_w->get_title());
                                db_card->container().append(new_db_task);
                                bind(new_db_task, task_w);

                                spdlog::get("app")->info(
                                    "(\"{}\") → New task \"{}\" has been "
                                    "appended to card \"{}\"",
                                    m_current_board->get_name(),
                                    new_db_task->get_name(),
                                    db_card->get_name());
                            } else {
                                auto& data = db_card->container().get_data();
                                auto new_db_task =
                                    Task::create(task_w->get_title());
                                data.insert(std::next(data.begin(), index + 1),
                                            new_db_task);
                                db_card->container().modify(true);

                                bind(new_db_task, task_w);

                                spdlog::get("app")->info(
                                    "(\"{}\") → New task \"{}\" has been "
                                    "inserted into card \"{}\"",
                                    m_current_board->get_name(),
                                    new_db_task->get_name(),
                                    db_card->get_name());
                            }
                        }));

                m_card_dialog_cnns.push_back(
                    card_dialog.signal_task_removed().connect(
                        [this, &card_dialog](ui::TaskWidget* task_w) {
                            auto db_card =
                                m_bound_cards[card_dialog.card_widget()];

                            std::shared_ptr<Task> to_remove =
                                m_bound_tasks[task_w];
                            db_card->container().remove(to_remove);
                            m_bound_tasks.erase(task_w);

                            spdlog::get("app")->info(
                                "(\"{}\") → Task \"{}\" has been removed from "
                                "card \"{}\"",
                                m_current_board->get_name(),
                                to_remove->get_name(), db_card->get_name());
                        }));

                m_card_dialog_cnns.push_back(
                    card_dialog.signal_task_reordered().connect(
                        [this, &card_dialog](ui::TaskWidget* next,
                                             ui::TaskWidget* sibling, bool up) {
                            auto db_card =
                                m_bound_cards[card_dialog.card_widget()];

                            if (up) {
                                db_card->container().reorder_before(
                                    m_bound_tasks[next],
                                    m_bound_tasks[sibling]);
                            } else {
                                db_card->container().reorder_after(
                                    m_bound_tasks[next],
                                    m_bound_tasks[sibling]);
                            }

                            spdlog::get("app")->info(
                                "(\"{}\") → In card \"{}\", Task \"{}\" has "
                                "been reordered {} "
                                "Task \"{}\"",
                                m_current_board->get_name(),
                                db_card->get_name(),
                                m_bound_tasks[next]->get_name(),
                                (up ? "before" : "after"),
                                m_bound_tasks[sibling]->get_name());
                        }));
            }));

        m_card_dialog_cnns.push_back(
            card_dialog.signal_closed().connect([this, &card_dialog]() {
                // The last two signals are the open and close connections,
                // which doesnt have to be deleted when a session is still on
                m_card_dialog_cnns.erase(
                    std::next(m_card_dialog_cnns.begin(), 2),
                    m_card_dialog_cnns.end());

                ui::CardWidget* card_w = card_dialog.card_widget();
                auto db_card = m_bound_cards[card_w];

                const std::string old_name = db_card->get_name();
                if (old_name != card_dialog.get_title()) {
                    card_w->set_title(card_dialog.get_title());

                    spdlog::get("app")->info(
                        "(\"{}\") → Card \"{}\" has been renamed to \"{}\"",
                        m_current_board->get_name(), old_name,
                        db_card->get_name());
                }

                Date date = card_dialog.get_deadline();

                if (date != db_card->get_due_date()) {
                    Date old_date = db_card->get_due_date();
                    db_card->set_due_date(date);

                    if (date.ok()) {
                        // It means the cards hasn't a deadline set until now,
                        // which means it should be added to the card updating
                        // queue too
                        if (!old_date.ok()) {
                            card_update_queue_push(card_w);

                            spdlog::get("app")->info(
                                "(\"{}\") → Card \"{}\"'s deadline has been "
                                "set",
                                m_current_board->get_name(),
                                db_card->get_name());
                        } else {
                            spdlog::get("app")->info(
                                "(\"{}\") → Card \"{}\"'s deadline has been "
                                "updated",
                                m_current_board->get_name(),
                                db_card->get_name());
                        }
                    } else {
                        // Date has been unset. We don't need to update this
                        // card's deadline label anymore
                        std::erase(m_cards, card_w);

                        spdlog::get("app")->info(
                            "(\"{}\") → Card \"{}\"'s deadline has been "
                            "unset",
                            m_current_board->get_name(), db_card->get_name());
                    }
                }

                if (date.ok() &&
                    (card_dialog.get_complete() != db_card->get_complete())) {
                    card_w->set_complete(card_dialog.get_complete());
                    card_w->update_deadline_label();
                    db_card->set_complete(card_dialog.get_complete());

                    spdlog::get("app")->info(
                        "(\"{}\") → Card \"{}\" has been marked as {}",
                        m_current_board->get_name(), db_card->get_name(),
                        (db_card->get_complete() ? "complete"
                                                 : "not complete"));
                }

                const std::pair<int, int> completion_ratio =
                    card_dialog.get_completion_ratio();
                card_w->set_completion_label(completion_ratio.second,
                                             completion_ratio.first);

                if (db_card->get_notes() != card_dialog.get_notes()) {
                    db_card->set_notes(card_dialog.get_notes());

                    spdlog::get("app")->info(
                        "(\"{}\") → Card \"{}\"'s notes has been updated",
                        m_current_board->get_name(), db_card->get_name());

                    card_w->set_notes_icon_visible(
                        !db_card->get_notes().empty());
                }

                spdlog::get("app")->info("(\"{}\") → Card Dialog closed for {}",
                                         m_current_board->get_name(),
                                         db_card->get_name());
            }));

        m_app_window.on_board_view();

        // TODO: Remove this method. There's no need for it
        setup_board_widget();

        m_idle_load_session_cnn = Glib::signal_idle().connect(
            sigc::mem_fun(*this, &AppContext::idle_load_session),
            Glib::PRIORITY_LOW);
        m_timeout_save_cnn = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_save_session),
            AppContext::SAVE_INTERVAL);
#if GTKMM_CHECK_VERSION(4, 12, 0)
        m_suspended_tracker_cnn =
            m_app_window.property_suspended().signal_changed().connect(
                sigc::mem_fun(*this, &AppContext::toggle_timeout_update_cards));
#else
        m_suspended_tracker_handler_id = g_signal_connect(
            m_app_window.gobj(), "notify::suspended",
            G_CALLBACK(+[](GtkWindow* self, GParamSpec* pspec, gpointer data) {
                reinterpret_cast<AppContext*>(data)
                    ->toggle_timeout_update_cards();
            }),
            this);
#endif
    } else {
        // Board has failed to load. Go back to previous state
        spdlog::get("app")->warn(
            "[AppContext.on_session_loaded] Board session initialization has "
            "failed");

        m_session_flags[Status::LOADING] = false;
        m_session_flags[Status::BUSY] = false;

        Gtk::AlertDialog::create(_("It was not possible to load this board"))
            ->show(m_app_window);
        m_app_window.on_main_menu();
    }
}

void AppContext::on_session_saved() {
    if (m_board_save_thread.joinable()) {
        m_board_save_thread.join();
        spdlog::get("app")->debug(
            "[AppContext.on_session_saved] Joining save worker thread");
    }

    m_app_window.set_spinner_visible(false);
    spdlog::get("app")->info("Changes made to Board (\"{}\") saved",
                             m_current_board->get_name());
}

#if GTKMM_CHECK_VERSION(4, 12, 0)
void AppContext::toggle_timeout_update_cards() {
    if (m_app_window.is_suspended()) {
        m_timeout_cards_update_cnn.disconnect();
    } else {
        m_timeout_cards_update_cnn = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_update_cards),
            AppContext::UPDATE_INTERVAL);
    }
}
#else
void AppContext::toggle_timeout_update_cards() {
    if (gtk_window_is_suspended(GTK_WINDOW(m_app_window.gobj()))) {
        m_timeout_cards_update_cnn.disconnect();
    } else {
        m_timeout_cards_update_cnn = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_update_cards),
            AppContext::UPDATE_INTERVAL);
    }
}
#endif

void AppContext::bind(const std::shared_ptr<Board>& db_board,
                      ui::BoardWidget* board_w) {
    m_board_widget_cnns.push_back(board_w->signal_name_changed().connect(
        [this, db_board](const std::string& old_name,
                         const std::string& new_name) {
            if (old_name != new_name) {
                db_board->set_name(new_name);

                spdlog::get("app")->info(
                    "(\"{}\") → Board \"{}\" has been renamed to \"{}\"",
                    m_current_board->get_name(), old_name, new_name);
            }
        }));

    m_board_widget_cnns.push_back(board_w->signal_background_changed().connect(
        [this, db_board](const std::string& old_background,
                         const std::string& new_background) {
            if (old_background != new_background) {
                BackgroundType bg_type =
                    Board::get_background_type(new_background);
                switch (bg_type) {
                    case BackgroundType::COLOR: {
                        db_board->set_background(
                            string_to_color(new_background));

                        spdlog::get("app")->info(
                            "(\"{}\") → Board background has been changed to "
                            "\"{}\"",
                            m_current_board->get_name(), new_background);
                        break;
                    }
                    case BackgroundType::IMAGE: {
                        db_board->set_background(new_background);

                        spdlog::get("app")->info(
                            "(\"{}\") → Board background has been changed to "
                            "\"{}\"",
                            m_current_board->get_name(), new_background);
                        break;
                    }
                    case BackgroundType::INVALID: {
                        spdlog::get("app")->warn(
                            "(\"{}\") → It was not possible to set the "
                            "background {}",
                            m_current_board->get_name(), new_background);
                        break;
                    }
                }
            }
        }));

    m_board_widget_cnns.push_back(board_w->signal_added_cardlist().connect(
        [this, db_board](ui::CardlistWidget* cardlist_w, int index) {
            if (index == -1) {
                std::shared_ptr<CardList> new_cardlist =
                    CardList::create(cardlist_w->get_name());
                db_board->container().append(new_cardlist);
                bind(new_cardlist, cardlist_w);

                spdlog::get("app")->info(
                    "(\"{}\") → Cardlist \"{}\" has been appended to Board",
                    m_current_board->get_name(), new_cardlist->get_name());
            } else {
                auto& inner_data = db_board->container().get_data();

                // TODO: Implement proper insert_index
                std::shared_ptr<CardList> new_cardlist =
                    CardList::create(cardlist_w->get_name());
                inner_data.insert(std::next(inner_data.begin(), index + 1),
                                  new_cardlist);
                db_board->container().modify(true);

                bind(new_cardlist, cardlist_w);

                spdlog::get("app")->info(
                    "(\"{}\") → Cardlist \"{}\" has been inserted to Board",
                    m_current_board->get_name(), new_cardlist->get_name());
            }
        }));

    m_board_widget_cnns.push_back(board_w->signal_remove_cardlist().connect(
        [this, db_board](ui::CardlistWidget* cardlist_w) {
            std::shared_ptr<CardList> to_remove = m_bound_cardlists[cardlist_w];
            db_board->container().remove(to_remove);

            m_bound_cardlists.erase(cardlist_w);

            spdlog::get("app")->info(
                "(\"{}\") → Cardlist \"{}\" has been removed from Board",
                m_current_board->get_name(), to_remove->get_name());
        }));

    m_board_widget_cnns.push_back(board_w->signal_reorder().connect(
        [this, db_board](ui::CardlistWidget* next, ui::CardlistWidget* sibling,
                         bool up) {
            if (up) {
                db_board->container().reorder_before(
                    m_bound_cardlists[next], m_bound_cardlists[sibling]);
            } else {
                db_board->container().reorder_after(m_bound_cardlists[next],
                                                    m_bound_cardlists[sibling]);
            }

            spdlog::get("app")->info(
                "(\"{}\") → Cardlist \"{}\" has been reordered {} Cardlist "
                "\"{}\"",
                m_current_board->get_name(),
                m_bound_cardlists[next]->get_name(), (up ? "before" : "after"),
                m_bound_cardlists[sibling]->get_name());
        }));
}

void AppContext::bind(const std::shared_ptr<CardList>& db_cardlist,
                      ui::CardlistWidget* cardlist_w) {
    m_bound_cardlists[cardlist_w] = db_cardlist;

    m_cardlists_cnns.push_back(cardlist_w->signal_name_changed().connect(
        [this, db_cardlist](std::string old_name, std::string new_name) {
            if (old_name != new_name) {
                db_cardlist->set_name(new_name);

                spdlog::get("app")->info(
                    "(\"{}\") → Cardlist \"{}\" has been renamed to \"{}\"",
                    m_current_board->get_name(), old_name, new_name);
            }
        }));

    m_cardlists_cnns.push_back(cardlist_w->signal_card_added().connect(
        [this, db_cardlist](ui::CardWidget* card_w, int index) {
            if (index == -1) {
                auto new_db_card = Card::create(card_w->get_title());
                db_cardlist->container().append(new_db_card);
                bind(new_db_card, card_w);

                spdlog::get("app")->info(
                    "(\"{}\") → New card \"{}\" has been appended onto "
                    "cardlist \"{}\"",
                    m_current_board->get_name(), new_db_card->get_name(),
                    db_cardlist->get_name());
            } else {
                auto& data = db_cardlist->container().get_data();
                auto new_db_card = Card::create(card_w->get_title());
                data.insert(std::next(data.begin(), index + 1), new_db_card);
                db_cardlist->container().modify(true);

                bind(new_db_card, card_w);

                spdlog::get("app")->info(
                    "(\"{}\") → New card \"{}\" has been inserted onto "
                    "cardlist \"{}\"",
                    m_current_board->get_name(), new_db_card->get_name(),
                    db_cardlist->get_name());
            }
        }));

    m_cardlists_cnns.push_back(cardlist_w->signal_card_removed().connect(
        [this, db_cardlist](ui::CardWidget* card_w) {
            std::shared_ptr<Card> to_remove = m_bound_cards[card_w];
            db_cardlist->container().remove(to_remove);
            m_bound_cards.erase(card_w);

            spdlog::get("app")->info(
                "(\"{}\") → Card \"{}\" has been removed from cardlist \"{}\"",
                m_current_board->get_name(), to_remove->get_name(),
                db_cardlist->get_name());
        }));

    m_cardlists_cnns.push_back(cardlist_w->signal_card_removed().connect(
        [this](ui::CardWidget* card_w) { std::erase(m_cards, card_w); }));

    m_cardlists_cnns.push_back(cardlist_w->signal_card_reorder().connect(
        [this, db_cardlist](ui::CardWidget* next, ui::CardWidget* sibling,
                            bool up) {
            if (up) {
                db_cardlist->container().reorder_before(m_bound_cards[next],
                                                        m_bound_cards[sibling]);
            } else {
                db_cardlist->container().reorder_after(m_bound_cards[next],
                                                       m_bound_cards[sibling]);
            }

            spdlog::get("app")->info(
                "(\"{}\") → Card \"{}\" has been reordered {} Card \"{}\"",
                m_current_board->get_name(), m_bound_cards[next]->get_name(),
                (up ? "before" : "after"), m_bound_cards[sibling]->get_name());
        }));

    m_cardlists_cnns.push_back(cardlist_w->signal_card_received().connect(
        [this, db_cardlist](ui::CardWidget* recv_widget,
                            ui::CardlistWidget* from_parent,
                            ui::CardWidget* sibling) {
            std::shared_ptr<CardList> received_from =
                m_bound_cardlists[from_parent];
            std::shared_ptr<Card> received_card = m_bound_cards[recv_widget];
            received_from->container().remove(received_card);

            if (!sibling) {
                db_cardlist->container().append(received_card);

                spdlog::get("app")->info(
                    "(\"{}\") → Card \"{}\" from cardlist \"{}\" has been "
                    "appended to cardlist \"{}\"",
                    m_current_board->get_name(), received_card->get_name(),
                    received_from->get_name(), db_cardlist->get_name());
            } else {
                db_cardlist->container().insert_after(received_card,
                                                      m_bound_cards[sibling]);

                spdlog::get("app")->info(
                    "(\"{}\") → Card \"{}\" from cardlist \"{}\" has been "
                    "inserted after card \"{}\" in cardlist \"{}\"",
                    m_current_board->get_name(), received_card->get_name(),
                    m_bound_cards[sibling]->get_name(),
                    received_from->get_name(), db_cardlist->get_name());
            }
        }));
}

void AppContext::bind(const std::shared_ptr<Card>& db_card,
                      ui::CardWidget* card_w) {
    m_bound_cards[card_w] = db_card;

    m_cards_cnns.push_back(card_w->signal_name_changed().connect(
        [this, db_card](const std::string& old_name,
                        const std::string& new_name) {
            if (old_name != new_name) {
                db_card->set_name(new_name);

                spdlog::get("app")->info(
                    "(\"{}\") → Card \"{}\" has been renamed to \"{}\"",
                    m_current_board->get_name(), old_name, new_name);
            }
        }));

    m_cards_cnns.push_back(card_w->signal_color_changed().connect(
        [this, db_card](const Gdk::RGBA old_color, const Gdk::RGBA new_color) {
            if (old_color != new_color) {
                db_card->set_color(
                    Color{new_color.get_red_u(), new_color.get_green_u(),
                          new_color.get_blue_u(), new_color.get_alpha()});

                spdlog::get("app")->info(
                    "(\"{}\") → Card \"{}\"'s color has been set to {}",
                    m_current_board->get_name(), db_card->get_name(),
                    new_color.to_string().c_str());
            }
        }));

    // FIXME: This callback will never be called! Remove the signal and this
    // handler
    m_cards_cnns.push_back(card_w->signal_card_received().connect(
        [this, db_card, card_w](ui::CardWidget* recv_widget,
                                ui::CardlistWidget* recv_from) {
            auto recv_from_cardlist = m_bound_cardlists[recv_from];
            auto recv_card = m_bound_cards[recv_widget];

            auto db_card_cardlist = m_bound_cardlists[card_w->parent()];

            recv_from_cardlist->container().signal_remove().block();
            recv_from_cardlist->container().remove(recv_card);
            recv_from_cardlist->container().signal_remove().unblock();

            // Insert to the new destination
            db_card_cardlist->container().insert_after(recv_card,
                                                       m_bound_cards[card_w]);
        }));
}

void AppContext::bind(const std::shared_ptr<Task>& db_task,
                      ui::TaskWidget* task_w) {
    m_bound_tasks[task_w] = db_task;

    m_tasks_cnns.push_back(task_w->signal_name_changed().connect(
        [this, db_task](const std::string& old_name,
                        const std::string& new_name) {
            if (old_name != new_name) {
                db_task->set_name(new_name);

                spdlog::get("app")->info(
                    "(\"{}\") → Task \"{}\" has been renamed to \"{}\"",
                    m_current_board->get_name(), old_name, new_name);
            }
        }));

    m_tasks_cnns.push_back(
        task_w->signal_complete_changed().connect([this, db_task, task_w]() {
            db_task->set_done(task_w->get_complete());

            spdlog::get("app")->info(
                "(\"{}\") → Task \"{}\" has been marked {}",
                m_current_board->get_name(), db_task->get_name(),
                task_w->get_complete() ? "complete" : "incomplete");
        }));
}

void AppContext::clear_binds() {
    m_board_widget_cnns.clear();
    m_cardlists_cnns.clear();
    m_cards_cnns.clear();
    m_card_dialog_cnns.clear();
}

void AppContext::card_update_queue_push(ui::CardWidget* card_w) {
    m_cards.push_back(card_w);

    if (m_timeout_cards_update_cnn.empty()) {
        m_timeout_cards_update_cnn = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_update_cards),
            UPDATE_INTERVAL);
    }
}

bool AppContext::on_window_closed() {
    if (!(m_session_flags[Status::CLEARING] ||
          m_session_flags[Status::LOADING]) &&
        m_session_flags[Status::BUSY]) {
        if (m_board_save_thread.joinable()) {
            spdlog::get("app")->debug(
                "[AppContext.on_window_closed] User requested window closing "
                "but a thread has been scheduled. join it");
            m_board_save_thread.join();
        } else {
            spdlog::get("app")->debug(
                "[AppContext.on_window_closed] User request window closing. "
                "Saving the session");
            m_manager.local_save(m_current_board);
        }
    }

    return false;
}

// FIXME
bool AppContext::idle_load_session() {
    if (!m_current_board) {
        spdlog::get("app")->error(
            "[AppContext.idle_load_session] Current board is invalid. Stopping "
            "loading task");
        m_session_flags[Status::LOADING] = false;
        return false;
    }

    if (m_session_flags[Status::CLEARING] && m_session_flags[Status::LOADING]) {
        spdlog::get("app")->warn(
            "[AppContext.idle_load_session] Board view is not clean. Current "
            "session (\"{}\") is waiting",
            m_current_board->get_name());
        return true;
    }

    const auto& data = m_current_board->container().get_data();
    // FIXME: This is logically confusing. m_cardlist_i gives the idea of an
    // indexer but it is actually working as a counter
    if (m_cardlist_i > (data.size() - 1) || data.empty()) {
        m_session_flags[Status::LOADING] = false;

        spdlog::get("app")->debug(
            "[AppContext.idle_load_session] Session (\"{}\") has been fully "
            "loaded",
            m_current_board->get_name());
        bind(m_current_board, &m_board_widget);

        m_timeout_cards_update_cnn = Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &AppContext::timeout_update_cards),
            AppContext::UPDATE_INTERVAL);
        m_idle_load_session_cnn.disconnect();
        return false;
    }

    ui::CardlistWidget* cardlist_widget = Gtk::make_managed<ui::CardlistWidget>(
        m_board_widget, data[m_cardlist_i]->get_name());
    m_board_widget.append(*cardlist_widget);

    for (const auto& card : data[m_cardlist_i]->container()) {
        ui::CardWidget* card_widget = builder_card_widget(card);
        if (card_widget->is_deadline_set()) {
            m_cards.push_back(card_widget);
        }
        cardlist_widget->append(*card_widget);
        bind(card, card_widget);
    }

    bind(data[m_cardlist_i], cardlist_widget);
    m_cardlist_i++;

    m_session_flags[Status::LOADING] = true;
    m_session_flags[Status::BUSY] = true;
    return true;
}

bool AppContext::idle_clear_session() {
    m_board_widget.pop();
    if (m_board_widget.empty()) {
        m_session_flags[Status::CLEARING] = false;
        m_session_flags[Status::BUSY] = false;

        spdlog::get("app")->debug(
            "[AppContext.idle_clear_session] Board view is free");
        return false;
    }

    m_session_flags[Status::BUSY] = true;
    return true;
}

bool AppContext::timeout_save_session() {
    if (!m_current_board) {
        // There is no board to save, stop this timeout procedure
        spdlog::get("app")->error(
            "[AppContext.timeout_save_session] Current board session is "
            "invalid and cannot be saved. Stopping saving task");
        return false;
    } else if (m_current_board->modified()) {
        if (m_board_save_thread.joinable()) {
            spdlog::get("app")->debug(
                "[AppContext.timeout_save_session] Cannot schedule new saver "
                "thread. Saver thread is still joinable");
            return true;
        } else {
            m_app_window.set_spinner_visible();
            m_board_save_thread = std::thread{[this]() {
                m_manager.local_save(m_current_board);
                m_save_board_dispatcher.emit();
            }};
            spdlog::get("app")->debug(
                "[AppContext.timeout_save_session] Saver worker thread has "
                "been scheduled");
            return true;
        }
    } else {
        spdlog::get("app")->debug(
            "[AppContext.timeout_save_session] No modifications registered. "
            "Don't schedule a saver "
            "worker thread");
        return true;
    }
}

bool AppContext::timeout_update_cards() {
    if (m_session_flags[Status::BUSY]) {
        const size_t size = m_cards.size();

        if (m_cards.empty()) {
            m_timeout_cards_update_cnn.disconnect();
            spdlog::get("app")->debug(
                "[AppContext.timeout_update_cards] Empty card queue. Stopping "
                "timeout update task.");
            return false;
        }

        if (m_next_card_i > size - 1) {
            m_next_card_i = 0;
        }

        auto card_w = m_cards[m_next_card_i];

        card_w->update_deadline_label();

        spdlog::get("app")->debug(
            "[AppContext.timeout_update_cards] CardWidget \"{}\"'s UI has "
            "been updated",
            card_w->get_title());

        m_next_card_i++;

        return true;
    }

    spdlog::get("app")->debug(
        "[AppContext.timeout_update_cards] There is no session running. Stop "
        "timeout update task.");
    return false;
}

void AppContext::setup_board_widget() {
    m_board_widget.set_name(m_current_board->get_name());
    m_board_widget.set_background(m_current_board->get_background());
}
