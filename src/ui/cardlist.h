#pragma once

#include <gtkmm.h>

#include <memory>

#include "../core/card.h"
#include "../core/cardlist.h"
#include "editable-label-header.h"
#include "card.h"

namespace ui {

#define CARDLIST_TITLE_FORMAT \
    "<span font-weight=\"bold\" size=\"large\">{}</span>"

class BoardWidget;

class CardWidget;

class CardListHeader : public EditableLabelHeader {
public:
    CardListHeader(std::shared_ptr<CardList>& cardlist_refptr);

    void set_label(std::string new_label);

protected:
    std::shared_ptr<CardList> cardlist_refptr;

    void on_confirm_changes();
};

class Cardlist : public Gtk::ListBox {
public:
    static const int CARDLIST_SIZE = 240;
    Cardlist(BoardWidget& board, std::shared_ptr<CardList> cardlist_refptr);

    ui::CardWidget* add_card(std::shared_ptr<Card> card_refptr);

    ui::CardWidget* add_card();

    void reorder_card(ui::CardWidget& next, ui::CardWidget& sibling);

    void append_card(ui::CardWidget* card);

    void remove_card(ui::CardWidget* card);

    std::shared_ptr<CardList>& get_cardlist_refptr();

    bool is_child(ui::CardWidget* card);

    CardListHeader& get_header();

private:
    CardListHeader cardlist_header;
    Gtk::Button add_card_button;
    Gtk::Box root;
    std::vector<ui::CardWidget*> cards_tracker;
    BoardWidget& board;
    std::shared_ptr<CardList> cardlist_refptr;
    int n_rows = 1; // We're counting with add_card_button

    void remove();
};
}  // namespace ui