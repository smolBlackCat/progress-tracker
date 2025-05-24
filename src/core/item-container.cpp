#include "item-container.h"

#include "cardlist.h"

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
ItemContainer<T>::ItemContainer() : m_data() {}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
std::shared_ptr<T> ItemContainer<T>::append(const T& item) {
    for (auto& i_item : m_data) {
        if (*i_item == item) {
            return nullptr;
        }
    }

    auto new_item = std::make_shared<T>(item);
    m_data.push_back(std::make_shared<T>(item));
    spdlog::get("core")->info("[ItemContainer] Item \"{}\" has been added",
                              item.get_name());
    modify();
    on_append_signal.emit(new_item);
    return m_data.back();
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
void ItemContainer<T>::remove(const T& item) {
    for (auto it = m_data.begin(); it != m_data.end(); it++) {
        if (item == *(*it)) {
            m_data.erase(it);
            spdlog::get("core")->info(
                "[ItemContainer] Item \"{}\" has been removed",
                item.get_name());
            modify();
            on_remove_signal.emit(*it);
            return;
        }
    }
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
std::shared_ptr<T> ItemContainer<T>::insert_after(const T& item,
                                                  const T& sibling) {
    return nullptr;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
std::shared_ptr<T> ItemContainer<T>::insert_before(const T& item,
                                                   const T& sibling) {
    return nullptr;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
ReorderingType ItemContainer<T>::reorder(const T& next, const T& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    ssize_t c = 0;
    for (auto& item : m_data) {
        if (*item == next) {
            next_i = c;
        } else if (*item == sibling) {
            sibling_i = c;
        }
        c++;
    }

    bool any_absent = next_i == -1 || sibling_i == -1;
    bool is_same = next_i == sibling_i;

    if (any_absent || is_same) {
        spdlog::get("core")->warn(
            "[ItemContainer] Cannot reorder items: same references or missing");
        return ReorderingType::INVALID;
    }

    std::shared_ptr<T> next_v = m_data[next_i];
    std::shared_ptr<T> sibling_v = m_data[sibling_i];

    m_data.erase(m_data.begin() + next_i);

    ReorderingType reordering;
    if (next_i > sibling_i) {
        // Down to up reordering
        m_data.insert(m_data.begin() + (sibling_i == 0 ? 0 : sibling_i),
                      next_v);
        reordering = ReorderingType::DOWNUP;
        spdlog::get("core")->info(
            "[ItemContainer] Item \"{}\" was inserted before Item \"{}\"",
            next.get_name(), sibling.get_name());
        modify();
        on_reorder_signal.emit(next_v, sibling_v, reordering);
        return reordering;
    } else if (next_i < sibling_i) {
        // Up to down reordering
        m_data.insert(m_data.begin() + sibling_i, next_v);
        reordering = ReorderingType::UPDOWN;
        spdlog::get("core")->info(
            "[ItemContainer] Item \"{}\" was inserted after Item \"{}\"",
            next.get_name(), sibling.get_name());
        modify();
        on_reorder_signal.emit(next_v, sibling_v, reordering);
        return reordering;
    }
    return ReorderingType::INVALID;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
std::vector<std::shared_ptr<T>>& ItemContainer<T>::get_data() {
    return m_data;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
const std::vector<std::shared_ptr<T>>& ItemContainer<T>::get_data() const {
    return m_data;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
std::vector<std::shared_ptr<T>>::iterator ItemContainer<T>::begin() {
    return m_data.begin();
}
template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
std::vector<std::shared_ptr<T>>::iterator ItemContainer<T>::end() {
    return m_data.end();
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
std::vector<std::shared_ptr<T>>::const_iterator ItemContainer<T>::begin()
    const {
    return m_data.begin();
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
std::vector<std::shared_ptr<T>>::const_iterator ItemContainer<T>::end() const {
    return m_data.end();
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
void ItemContainer<T>::modify(bool m) {
    m_modified = m;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
bool ItemContainer<T>::modified() const {
    for (auto& item : m_data) {
        if (item->modified()) return true;
    }
    return m_modified;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
sigc::signal<void(std::shared_ptr<T>)>& ItemContainer<T>::signal_append() {
    return on_append_signal;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
sigc::signal<void(std::shared_ptr<T>)>& ItemContainer<T>::signal_remove() {
    return on_remove_signal;
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
sigc::signal<void(std::shared_ptr<T>, std::shared_ptr<T>, ReorderingType)>&
ItemContainer<T>::signal_reorder() {
    return on_reorder_signal;
}

template class ItemContainer<CardList>;
template class ItemContainer<Card>;
template class ItemContainer<Task>;
