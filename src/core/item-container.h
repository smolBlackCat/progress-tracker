#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "item.h"
#include "modifiable.h"

enum class ReorderingType {
    AFTER,
    BEFORE,
    INVALID,
};

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
    virtual void append(std::shared_ptr<T>& item);

    /**
     * @brief Removes an item from the container.
     *
     * @details The method attempts to remove the corresponding shared_ptr
     * holding the same value given. When the item is not found, the method
     * does nothing.
     *
     * @param item The item to remove.
     */
    virtual void remove(std::shared_ptr<T>& item);

    virtual void insert_after(std::shared_ptr<T>& item,
                              std::shared_ptr<T>& sibling);
    virtual void insert_before(std::shared_ptr<T>& item,
                               std::shared_ptr<T>& sibling);

    virtual void reorder_after(std::shared_ptr<T>& next,
                               std::shared_ptr<T>& sibling);

    virtual void reorder_before(std::shared_ptr<T>& next,
                                std::shared_ptr<T>& sibling);

    void modify(bool m = true) override;

    ssize_t size() const;
    
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
