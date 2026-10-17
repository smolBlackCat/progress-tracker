#pragma once

#include <gtkmm.h>
#include <spdlog/spdlog.h>

/**
 * @brief Gtk::Box helper class
 *
 * Specialize common addition, reordering and deletion of elements through a
 * wrapper
 */
template <typename ItemT>
class OrderedItemBox {
public:
    explicit OrderedItemBox(Gtk::Box& box,
                            Gtk::Widget* trailing_widget = nullptr);

    /**
     * @brief Appends an item. If there is a trailing-widget defined, append
     * will always insert item before the trailing widget
     *
     * @param item child widget
     */
    void append(ItemT& item);

    void insert_after(ItemT& item, ItemT& sibling);
    void remove(ItemT& item);
    void reorder_after(ItemT& item, ItemT& sibling);
    void reorder_before(ItemT& item, ItemT& sibling);

    sigc::signal<void(ItemT*, int)>& signal_item_added();
    sigc::signal<void(ItemT*)>& signal_item_removed();
    sigc::signal<void(ItemT*, ItemT*, bool)>& signal_item_reordered();

protected:
    Gtk::Box& m_box;
    Gtk::Widget* m_trailing_widget = nullptr;

private:
    sigc::signal<void(ItemT*, int)> m_item_added_signal;
    sigc::signal<void(ItemT*)> m_item_removed_signal;
    sigc::signal<void(ItemT*, ItemT*, bool)> m_item_reordered_signal;
};

/**
 * @brief Gtk::Box helper class
 *
 * Aside from the basic implementation of element addition, reordering and
 * removal, this wrapper takes into consideration child widgets that need to be
 * transferred from a parent to another without getting destroyed (deallocated)
 * in the process
 */
template <typename ItemT>
class TransferableItemBox : public OrderedItemBox<ItemT> {
public:
    using OrderedItemBox<ItemT>::OrderedItemBox;

    void receive_from(OrderedItemBox<ItemT>& source, ItemT& item,
                      ItemT* sibling = nullptr);

    sigc::signal<void(ItemT*, OrderedItemBox<ItemT>*, ItemT*)>&
    signal_item_received();

private:
    sigc::signal<void(ItemT*, OrderedItemBox<ItemT>*, ItemT*)>
        m_item_received_signal;
};