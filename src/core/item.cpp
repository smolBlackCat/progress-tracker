#include "item.h"

#include <guid.hpp>

extern const std::shared_ptr<spdlog::logger> core_logger =
    spdlog::stdout_color_mt("core");

Item::Item(const std::string& name) : Item{name, xg::newGuid()} {}

Item::Item(const std::string& name, xg::Guid uuid) : name{name}, uuid{uuid} {}

Item::~Item() {}

void Item::set_name(const std::string& other) {
    if (!name.empty())
        spdlog::get("core")->info("[Item] Item \"{}\" renamed to \"{}\"", name,
                                  other);

    name = other;
    name_changed.emit();
}

std::string Item::get_name() const { return name; }

bool Item::operator==(const Item& item) const {
    return item.get_name() == name && item.uuid == uuid;
}

xg::Guid Item::get_id() const { return uuid; }

sigc::signal<void()>& Item::signal_name_changed() { return name_changed; }