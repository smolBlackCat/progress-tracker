#pragma once

#include <gtkmm.h>

#include <memory>

#include "../core/card.h"
#include "../core/cardlist.h"
#include "board-widget.h"
#include "card.h"
#include "editable-label-header.h"

namespace ui {

class BoardWidget;

class CardWidget;

class CardListHeader : public EditableLabelHeader {
public:
    CardListHeader(std::shared_ptr<CardList>& cardlist_refptr);

protected:
    std::shared_ptr<CardList> cardlist_refptr;

    void on_confirm_changes() override;
};

class CardlistWidget : public Gtk::ListBox {
public:
    static const int CARDLIST_SIZE = 240;

    CardlistWidget(BoardWidget& board,
                   std::shared_ptr<CardList> cardlist_refptr);
    ui::CardWidget* add_card(std::shared_ptr<Card> card_refptr);
    ui::CardWidget* add_card();
    void remove_card(ui::CardWidget* card);
    std::shared_ptr<CardList>& get_cardlist_refptr();
    bool is_child(ui::CardWidget* card);
    CardListHeader& get_header();

private:
    // Widgets
    CardListHeader cardlist_header;
    Gtk::Button add_card_button;
    Gtk::Box root;

    // Data
    BoardWidget& board;
    std::shared_ptr<CardList> cardlist_refptr;
    std::vector<ui::CardWidget*> cards_tracker;

    void reorder_card(ui::CardWidget& next, ui::CardWidget& sibling);

    void setup_drag_and_drop(ui::CardWidget* card);
};
}  // namespace ui