#pragma once

#include <gtkmm.h>

#include <memory>

#include "../core/card.h"
#include "cardlist-widget.h"
#include "editable-label-header.h"

namespace ui {

class CardlistWidget;

/**
 * @brief Card widget that allows the user to make some modifications like
 * renaming and removing.
 */
class CardWidget : public EditableLabelHeader {
public:
    /**
     * @brief CardWidget constructor
     *
     * @param card_ptr a smart pointer pointing to a Card object.
     */
    CardWidget(std::shared_ptr<Card> card_refptr, bool is_new = false);

    /**
     * @brief Removes itself from the associated CardlistWidget object.
     */
    void remove();

    /**
     * @brief Sets a new parent to this card.
     *
     * @param cardlist_p pointer to a new CardlistWidget object (parent)
     *
     * @details Cards in Progress Tracker are the only ones that changes parents
     *          with a certain frequency, that's why this method exists.
     */
    void set_cardlist(ui::CardlistWidget* cardlist_p);

    std::shared_ptr<Card> get_card();

protected:
    bool is_new;
    void on_confirm_changes() override;
    void on_cancel_changes() override;

private:
    std::shared_ptr<Card> card_refptr;
    ui::CardlistWidget* cardlist_p;
};
}  // namespace ui