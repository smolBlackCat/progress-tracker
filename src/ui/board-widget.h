#pragma once

#include <gtkmm.h>

#include <memory>
#include <vector>

#include "../core/board.h"
#include "cardlist.h"

#define CSS_FORMAT \
    "#board-root {transition-property: background-image, background-color;}"
#define CSS_FORMAT_FILE                                                       \
    "#board-root {{transition-property: background-image, background-color; " \
    "background-image: url(file:{}); background-position: center;}}"
#define CSS_FORMAT_RGB                                                        \
    "#board-root {{transition-property: background-image, background-color; " \
    "background-color: {};}}"

namespace ui {

class BoardWidget;

class BoardWidget : public Gtk::ScrolledWindow {
public:
    BoardWidget();

    ~BoardWidget() override;

    void set(Board* board);

    void clear();

    bool save();

    /**
     * @brief Creates a new Cardlist widget along with a CardList data model.
     */
    void add_cardlist(std::shared_ptr<CardList> cardlist_refptr);

    bool remove_cardlist(ui::Cardlist& cardlist);

private:
    Gtk::Box root;
    Gtk::Button add_button;
    Board* board;
    Glib::RefPtr<Gtk::CssProvider> css_provider_refptr;
    std::vector<ui::Cardlist*> cardlist_vector;

    bool set_background();
};
}  // namespace ui