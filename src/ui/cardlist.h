#pragma once

#include <gtkmm.h>

#include <memory>

#include "../core/card.h"
#include "../core/cardlist.h"
#include "card.h"
#include "editable-label-header.h"

namespace ui {

#define CARDLIST_TITLE_FORMAT \
    "<span font-weight=\"bold\" size=\"large\">{}</span>"

#define CARDLIST_STYLE \
    "list.rich-list {{transition-property: opacity; transition-duration: 0.5s; border-radius: 20px; opacity: {};}}"

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