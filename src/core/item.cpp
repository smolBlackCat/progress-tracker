#include "item.h"

#include <guid.hpp>

extern const std::shared_ptr<spdlog::logger> core_logger =
    spdlog::stdout_color_mt("core");

Item::Item(const std::string& name)
    : Modifiable{}, name{name}, uuid{xg::newGuid()} {}

Item::Item(const std::string& name, xg::Guid uuid)
    : Modifiable{}, name{name}, uuid{uuid} {}

void Item::set_name(const std::string& other) {
    if (!name.empty())
        spdlog::get("core")->info("[Item] Item \"{}\" renamed to \"{}\"", name,
                                  other);

    name = other;
    set_modified();
}

const std::string& Item::get_name() const { return name; }

bool Item::operator==(const Item& item) const {
    return item.get_name() == name && item.uuid == uuid;
}

xg::Guid Item::get_id() const { return uuid; }