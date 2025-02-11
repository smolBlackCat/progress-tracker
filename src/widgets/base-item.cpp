#include "base-item.h"

namespace ui {
BaseItem::BaseItem(Gtk::Orientation orientation, int spacing) : Gtk::Widget{} {
    set_layout_manager(Gtk::BoxLayout::create(orientation));

    static_cast<Gtk::BoxLayout*>(get_layout_manager().get())
        ->set_spacing(spacing);

    signal_destroy().connect(sigc::mem_fun(*this, &BaseItem::cleanup));
}

BaseItem::~BaseItem() {
    if (!gobj()) {
        return;
    }

    cleanup();
}

int BaseItem::get_n_visible_children() const {
    int n_children = 0;
    for (const Widget* child = get_first_child(); child;
         child = child->get_next_sibling()) {
        if (child->get_visible()) ++n_children;
    }
    return n_children;
}

Gtk::SizeRequestMode BaseItem::get_request_mode_vfunc() {
    return Gtk::SizeRequestMode::HEIGHT_FOR_WIDTH;
}

void BaseItem::measure_vfunc(Gtk::Orientation orientation, int for_size,
                             int& minimum, int& natural, int& minimum_baseline,
                             int& natural_baseline) const {
    // Don't use baseline alignment.
    minimum_baseline = -1;
    natural_baseline = -1;

    minimum = 0;
    natural = 0;

    // Number of visible children.
    const int nvis_children = get_n_visible_children();

    if (orientation == Gtk::Orientation::HORIZONTAL) {
        // Divide the height equally among the visible children.
        if (for_size > 0 && nvis_children > 0) for_size /= nvis_children;

        // Request a width equal to the width of the widest visible child.
    }

    for (const Widget* child = get_first_child(); child;
         child = child->get_next_sibling())
        if (child->get_visible()) {
            int child_minimum, child_natural, ignore;
            child->measure(orientation, for_size, child_minimum, child_natural,
                           ignore, ignore);
            minimum = std::max(minimum, child_minimum);
            natural = std::max(natural, child_natural);
        }

    if (orientation == Gtk::Orientation::VERTICAL) {
        // The allocated height will be divided equally among the visible
        // children. Request a height equal to the number of visible
        // children times the height of the highest child.
        minimum *= nvis_children;
        natural *= nvis_children;
    }
}

void BaseItem::size_allocate_vfunc(int width, int height, int baseline) {
    // Do something with the space that we have actually been given:
    //(We will not be given heights or widths less than we have requested,
    // though we might get more.)

    // Number of visible children.
    const int nvis_children = get_n_visible_children();

    if (nvis_children <= 0) {
        // No visible child.
        return;
    }

    // Assign space to the children:
    Gtk::Allocation child_allocation;
    const int height_per_child = height / nvis_children;

    // Place the first visible child at the top-left:
    child_allocation.set_x(0);
    child_allocation.set_y(0);

    // Make it take up the full width available:
    child_allocation.set_width(width);
    child_allocation.set_height(height_per_child);

    // Divide the height equally among the visible children.
    for (Widget* child = get_first_child(); child;
         child = child->get_next_sibling())
        if (child->get_visible()) {
            child->size_allocate(child_allocation, baseline);
            child_allocation.set_y(child_allocation.get_y() + height_per_child);
        }
}

void BaseItem::cleanup() {}
}  // namespace ui