#include "item-container.h"

#include "cardlist.h"

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
ItemContainer<T>::ItemContainer() : Modifiable{}, data() {}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
std::shared_ptr<T> ItemContainer<T>::append(const T& item) {
    for (auto& i_item : data) {
        if (*i_item == item) {
            return nullptr;
        }
    }

    data.push_back(std::make_shared<T>(item));
    set_modified();
    return data.back();
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
void ItemContainer<T>::remove(const T& item) {
    for (auto it = data.begin(); it != data.end(); it++) {
        if (item == *(*it)) {
            data.erase(it);
            set_modified();
            return;
        }
    }
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
std::shared_ptr<T> ItemContainer<T>::insert_after(const T& item,
                                                  const T& sibling) {
    return nullptr;
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
std::shared_ptr<T> ItemContainer<T>::insert_before(const T& item,
                                                   const T& sibling) {
    return nullptr;
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
ReorderingType ItemContainer<T>::reorder(const T& next, const T& sibling) {
    ssize_t next_i = -1;
    ssize_t sibling_i = -1;

    ssize_t c = 0;
    for (auto& item : data) {
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
            "[Board] Cannot reorder cardlists: same references or missing");
        return ReorderingType::INVALID;
    }

    std::shared_ptr<T> next_v = data[next_i];
    data.erase(data.begin() + next_i);

    ReorderingType reordering;
    if (next_i > sibling_i) {
        // Down to up reordering
        data.insert(data.begin() + (sibling_i == 0 ? 0 : sibling_i), next_v);
        reordering = ReorderingType::DOWNUP;
        spdlog::get("core")->info(
            "[Board] CardList \"{}\" was inserted before CardList \"{}\"",
            next.get_name(), sibling.get_name());
        set_modified();
        return reordering;
    } else if (next_i < sibling_i) {
        // Up to down reordering
        data.insert(data.begin() + sibling_i, next_v);
        reordering = ReorderingType::UPDOWN;
        spdlog::get("core")->info(
            "[Board] CardList \"{}\" was inserted after CardList \"{}\"",
            next.get_name(), sibling.get_name());
        set_modified();
        return reordering;
    }
    return ReorderingType::INVALID;
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
std::vector<std::shared_ptr<T>>& ItemContainer<T>::get_data() {
    return data;
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
const std::vector<std::shared_ptr<T>>& ItemContainer<T>::get_data() const {
    return data;
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
std::vector<std::shared_ptr<T>>::iterator ItemContainer<T>::begin() {
    return data.begin();
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
std::vector<std::shared_ptr<T>>::iterator ItemContainer<T>::end() {
    return data.end();
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
std::vector<std::shared_ptr<T>>::const_iterator ItemContainer<T>::begin()
    const {
    return data.begin();
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
std::vector<std::shared_ptr<T>>::const_iterator ItemContainer<T>::end() const {
    return data.end();
}

template <typename T>
    requires std::is_copy_constructible_v<T> && std::is_base_of_v<Item, T>
bool ItemContainer<T>::get_modified() const {
    for (auto& item : data) {
        if (item->get_modified()) return true;
    }
    return modified;
}

template class ItemContainer<CardList>;
template class ItemContainer<Card>;
template class ItemContainer<Task>;
