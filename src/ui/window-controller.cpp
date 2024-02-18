#include "window-controller.h"

#include <filesystem>

#include "../core/board.h"
#include "preferences-board-dialog.h"
#include "board-card-button.h"
#include "window.h"

namespace ui {

WindowController::WindowController(
    const Glib::RefPtr<Gtk::Builder>& window_builder, Gtk::Window& cb_dialog,
    Gtk::Window& pref_dialog, Gtk::Window& about_dialog,
    BoardWidget& board_widget, DeleteBoardsBar& delete_boards_bar)
    : window_builder{window_builder},
      cb_dialog{cb_dialog},
      pref_dialog{pref_dialog},
      about_dialog{about_dialog},
      board_widget{board_widget},
      delete_boards_bar{delete_boards_bar} {}

void WindowController::add_board(std::string board_filepath) {
    auto new_board_card = Gtk::make_managed<BoardCardButton>(board_filepath);
    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
        ->insert(*new_board_card, 0);
    auto fb_child_p = window_builder->get_widget<Gtk::FlowBox>("boards-grid")
                          ->get_child_at_index(0);
    new_board_card->signal_clicked().connect(
        [this, new_board_card, fb_child_p]() {
            if (!this->on_delete_mode) {
                Board* board = new Board{new_board_card->get_filepath()};
                on_board_view();
                board_widget.set(board, new_board_card);;
                set_title(board->get_name());
            } else {
                if (fb_child_p->is_selected()) {
                    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
                        ->unselect_child(*fb_child_p);
                } else {
                    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
                        ->select_child(*fb_child_p);
                }
            }
        });
}

void WindowController::show_about_dialog() { about_dialog.set_visible(); }

void WindowController::show_create_board_dialog() { cb_dialog.set_visible(); }

void WindowController::on_delete_board_mode() {
    on_delete_mode = true;
    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
        ->set_selection_mode(Gtk::SelectionMode::MULTIPLE);
    delete_boards_bar.set_reveal_child();
}

void WindowController::off_delete_board_mode() {
    on_delete_mode = false;
    delete_boards_bar.set_reveal_child(false);
    window_builder->get_widget<Gtk::FlowBox>("boards-grid")
        ->set_selection_mode(Gtk::SelectionMode::NONE);
}

void WindowController::on_main_menu() {
    window_builder->get_widget<Gtk::Stack>("app-stack")
        ->set_visible_child("board-grid-page");
    window_builder->get_widget<Gtk::MenuButton>("app-menu-button")
        ->set_menu_model(
            window_builder->get_object<Gio::MenuModel>("board-grid-menu"));
    window_builder->get_widget<Gtk::Button>("home-button")->set_visible(false);
    window_builder->get_widget<Gtk::Button>("add-board-button")->set_visible();
    set_title("Progress");
    board_widget.save();
}

void WindowController::on_board_view() {
    window_builder->get_widget<Gtk::Stack>("app-stack")
        ->set_visible_child("board-page");
    window_builder->get_widget<Gtk::MenuButton>("app-menu-button")
        ->set_menu_model(
            window_builder->get_object<Gio::MenuModel>("board-menu"));
    window_builder->get_widget<Gtk::Button>("home-button")->set_visible();
    window_builder->get_widget<Gtk::Button>("add-board-button")
        ->set_visible(false);
}

void WindowController::delete_selected_boards() {
    auto boards_grid = window_builder->get_widget<Gtk::FlowBox>("boards-grid");
    auto selected_children = boards_grid->get_selected_children();
    for (auto& fb_child_p : selected_children) {
        ui::BoardCardButton* cur_child =
            (ui::BoardCardButton*)fb_child_p->get_child();
        std::filesystem::remove(cur_child->get_filepath());
        boards_grid->remove(*cur_child);
    }

    off_delete_board_mode();
}

void WindowController::set_title(std::string title) {
    window_builder->get_widget<Gtk::Window>("app-window")->set_title(title);
}
}  // namespace ui