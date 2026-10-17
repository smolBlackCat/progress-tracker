#include "ordered-item-box.h"

#include "widgets/card-widget.h"
#include "widgets/cardlist-widget.h"
#include "widgets/task-widget.h"

template <typename ItemT>
OrderedItemBox<ItemT>::OrderedItemBox(Gtk::Box& box,
                                      Gtk::Widget* trailing_widget)
    : m_box(box), m_trailing_widget(trailing_widget) {}

template <typename ItemT>
void OrderedItemBox<ItemT>::append(ItemT& item) {
    m_box.append(item);

    if (m_trailing_widget) {
        m_box.reorder_child_after(*m_trailing_widget, item);
    }

    m_item_added_signal.emit(&item, -1);
}

template <typename ItemT>
void OrderedItemBox<ItemT>::insert_after(ItemT& item, ItemT& sibling) {
    if (sibling.get_parent() == &m_box && item.get_parent() == nullptr) {
        int index = 0;
        for (Gtk::Widget* widget = m_box.get_first_child();
             widget && widget != &sibling;
             widget = widget->get_next_sibling()) {
            index++;
        }

        m_box.insert_child_after(item, sibling);
        m_item_added_signal.emit(&item, index);
    }
}

template <typename ItemT>
void OrderedItemBox<ItemT>::remove(ItemT& item) {
    m_item_removed_signal.emit(&item);
    m_box.remove(item);
}

template <typename ItemT>
void OrderedItemBox<ItemT>::reorder_after(ItemT& item, ItemT& sibling) {
    if (item.get_parent() && sibling.get_parent() &&
        item.get_parent() == sibling.get_parent()) {
        if (sibling.get_next_sibling() != &item) {
            m_box.reorder_child_after(item, sibling);
            m_item_reordered_signal(&item, &sibling, false);
        }
    }
}

template <typename ItemT>
void OrderedItemBox<ItemT>::reorder_before(ItemT& item, ItemT& sibling) {
    if (item.get_parent() && sibling.get_parent() &&
        item.get_parent() == sibling.get_parent()) {
        if (sibling.get_prev_sibling() != &item) {
            item.insert_before(m_box, sibling);
            m_item_reordered_signal(&item, &sibling, true);
        }
    }
}

template <typename ItemT>
sigc::signal<void(ItemT*, int)>& OrderedItemBox<ItemT>::signal_item_added() {
    return m_item_added_signal;
}

template <typename ItemT>
sigc::signal<void(ItemT*)>& OrderedItemBox<ItemT>::signal_item_removed() {
    return m_item_removed_signal;
}

template <typename ItemT>
sigc::signal<void(ItemT*, ItemT*, bool)>&
OrderedItemBox<ItemT>::signal_item_reordered() {
    return m_item_reordered_signal;
}

template <typename ItemT>
void TransferableItemBox<ItemT>::receive_from(OrderedItemBox<ItemT>& source,
                                              ItemT& item, ItemT* sibling) {
    item.reference();

    source.signal_item_removed().block();
    source.remove(item);

    this->m_box.append(item);

    if (this->m_trailing_widget) {
        this->m_box.reorder_child_after(*this->m_trailing_widget, item);
    }

    if (sibling) {
        this->m_box.reorder_child_after(item, *sibling);
    }

    source.signal_item_removed().unblock();
    item.unreference();

    m_item_received_signal.emit(&item, &source, sibling);
}

template <typename ItemT>
sigc::signal<void(ItemT*, OrderedItemBox<ItemT>*, ItemT*)>&
TransferableItemBox<ItemT>::signal_item_received() {
    return m_item_received_signal;
}

template class OrderedItemBox<ui::TaskWidget>;
template class OrderedItemBox<ui::CardlistWidget>;
template class OrderedItemBox<ui::CardWidget>;
template class TransferableItemBox<ui::CardWidget>;