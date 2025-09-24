#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "item.h"
#include "modifiable.h"

/**
 * @brief ItemContainer is a template class that implements a set of behaviours
 * pertinent to container of items.
 */
template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
class ItemContainer : public Modifiable {
public:
    ItemContainer();
    virtual ~ItemContainer() = default;

    /**
     * @brief Appends an item to the container.
     *
     * @details The method roughly takes a copy of the given object and wraps it
     * in a shared pointer so it can be easily managed without worrying about
     * reference invalidation.
     *
     * @param item The item to append.
     *
     * @return A shared pointer to the appended item.
     */
    virtual std::shared_ptr<T> append(const T& item);

    /**
     * @brief Removes an item from the container.
     *
     * @details The method attempts to remove the corresponding shared_ptr
     * holding the same value given. When the item is not found, the method
     * does nothing.
     *
     * @param item The item to remove.
     */
    virtual void remove(const T& item);

    /**
     * @brief Inserts an item after a sibling item.
     *
     * @param item The item to insert.
     * @param sibling The sibling item after which to insert the item.
     */
    virtual std::shared_ptr<T> insert_after(const T& item, const T& sibling);

    /**
     * @brief Inserts an item before a sibling item.
     *
     * @param item The item to insert.
     * @param sibling The sibling item before which to insert the item.
     */
    virtual std::shared_ptr<T> insert_before(const T& item, const T& sibling);

    /**
     * @brief Reorders an item and sibling item.
     *
     * @details The reordering performed depends on whether the item in next
     * comes before or after the sibling item. If the item in next comes before
     * the sibling item, the item is moved to the position before the sibling
     * item. If the item in next comes after the sibling item, the item is moved
     * to the position after the sibling item.
     *
     * @param next The item to reorder.
     * @param sibling The sibling item after or before which to reorder the
     * item.
     *
     * @return The type of reordering performed.
     */
    virtual ReorderingType reorder(const T& next, const T& sibling);

    /**
     * @brief Returns a reference to the container's data.
     *
     * @return A reference to the container's data.
     */
    std::vector<std::shared_ptr<T>>& get_data();

    /**
     * @brief Returns a const reference to the container's data.
     *
     * @return A const reference to the container's data.
     */
    const std::vector<std::shared_ptr<T>>& get_data() const;

    void modify(bool m = true) override;

    /**
     * @brief Returns the container's modified state.
     *
     * The container is considered modified if the following conditions are met:
     * - Operations that naturally modify the container's data. (e.g., adding,
     * removing, reordering etc.)
     * - The container's items have been directly modified.
     *
     * @return True if the container has been modified, false otherwise.
     */
    bool modified() const override;

    std::vector<std::shared_ptr<T>>::iterator begin();
    std::vector<std::shared_ptr<T>>::iterator end();

    std::vector<std::shared_ptr<T>>::const_iterator begin() const;
    std::vector<std::shared_ptr<T>>::const_iterator end() const;

    sigc::signal<void(std::shared_ptr<T>)>& signal_append();
    sigc::signal<void(std::shared_ptr<T>)>& signal_remove();
    sigc::signal<void(std::shared_ptr<T>, std::shared_ptr<T>, ReorderingType)>&
    signal_reorder();

protected:
    std::vector<std::shared_ptr<T>> m_data;
    bool m_modified = false;

    // Signals
    sigc::signal<void(std::shared_ptr<T>)> on_append_signal;
    sigc::signal<void(std::shared_ptr<T>)> on_remove_signal;
    sigc::signal<void(std::shared_ptr<T>, std::shared_ptr<T>, ReorderingType)>
        on_reorder_signal;
};
