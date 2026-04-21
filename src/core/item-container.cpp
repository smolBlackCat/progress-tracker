#include "item-container.h"

#include "cardlist.h"

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
ItemContainer<T>::ItemContainer() : m_data() {}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
void ItemContainer<T>::append(std::shared_ptr<T>& item) {
    for (auto& i_item : m_data) {
        if (i_item == item) {
            return;
        }
    }

    m_data.push_back(item);
    modify();
    on_append_signal.emit(item);
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
void ItemContainer<T>::remove(std::shared_ptr<T>& item) {
    ssize_t n = std::erase(m_data, item);

    if (n >= 1) {
        modify();
        on_remove_signal.emit(item);
    }
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
void ItemContainer<T>::insert_after(std::shared_ptr<T>& item,
                                    std::shared_ptr<T>& sibling) {
    ssize_t index = -1;
    ssize_t c = 0;
    for (const auto& item : m_data) {
        if (item == sibling) {
            index = c;
            break;
        }

        c++;
    }

    if (index == -1) return;

    if (index == (m_data.size() - 1)) {
        m_data.push_back(item);
    } else {
        auto sibling_it = (m_data.begin() + index + 1);
        m_data.insert(sibling_it, item);
    }
    modify();
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
void ItemContainer<T>::insert_before(std::shared_ptr<T>& item,
                                     std::shared_ptr<T>& sibling) {
    ssize_t index = -1;
    ssize_t c = 0;
    for (const auto& item : m_data) {
        if (item == sibling) {
            index = c;
            break;
        }

        c++;
    }

    if (index == -1) return;

    auto sibling_it = (m_data.begin() + index);

    m_data.insert(sibling_it, item);
    modify();
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
void ItemContainer<T>::reorder_after(std::shared_ptr<T>& next,
                                     std::shared_ptr<T>& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    ssize_t c = 0;
    for (auto& item : m_data) {
        if (item == next) {
            next_i = c;
        } else if (item == sibling) {
            sibling_i = c;
        }
        c++;
    }

    bool any_absent = next_i == -1 || sibling_i == -1;
    bool is_same = next_i == sibling_i;
    bool already_in_place = next_i > sibling_i;

    if (any_absent || is_same || already_in_place) {
        on_reorder_signal.emit(next, sibling, ReorderingType::INVALID);
        return;
    }

    std::shared_ptr<T> temp = next;
    std::erase(m_data, next);

    m_data.insert(std::next(m_data.begin(), sibling_i), next);
    modify();
    on_reorder_signal.emit(next, sibling, ReorderingType::AFTER);
}

template <typename T>
    requires std::is_base_of_v<Item, T> && std::is_base_of_v<Modifiable, T>
void ItemContainer<T>::reorder_before(std::shared_ptr<T>& next,
                                      std::shared_ptr<T>& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    ssize_t c = 0;
    for (auto& item : m_data) {
        if (item == next) {
            next_i = c;
        } else if (item == sibling) {
            sibling_i = c;
        }
        c++;
    }

    bool any_absent = next_i == -1 || sibling_i == -1;
    bool is_same = next_i == sibling_i;
    bool already_in_place = next_i < sibling_i;

    if (any_absent || is_same || already_in_place) {
        on_reorder_signal.emit(next, sibling, ReorderingType::INVALID);
        return;
    }

    std::shared_ptr<T> temp = next;
    std::erase(m_data, next);

    m_data.insert(std::next(m_data.begin(), sibling_i), next);
    modify();
    on_reorder_signal.emit(next, sibling, ReorderingType::BEFORE);
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
ssize_t ItemContainer<T>::size() const {
    return m_data.size();
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
