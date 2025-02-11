#pragma once

#include <gtkmm.h>

namespace ui {
/**
 * @brief Helper base class for representing items in an application. This is
 * meant to be a simple implementation of a box like widget, except that it
 * allows to be customised along with a Glib::ExtraClassInit class
 */
class BaseItem : public Gtk::Widget {
public:
    BaseItem();
    virtual ~BaseItem();

protected:
    /**
     * @brief Retrieves the number of visible children in the root box.
     *
     * @return the number of visible children.
     */
    int get_n_visible_children() const;

    /**
     * @brief Retrieves the size request mode.
     *
     * @return the size request mode.
     */
    Gtk::SizeRequestMode get_request_mode_vfunc();

    /**
     * @brief Measures the widget size.
     *
     * @param orientation the orientation to measure.
     * @param for_size the size to measure for.
     * @param minimum the minimum size.
     * @param natural the natural size.
     * @param minimum_baseline the minimum baseline.
     * @param natural_baseline the natural baseline.
     */
    void measure_vfunc(Gtk::Orientation orientation, int for_size, int& minimum,
                       int& natural, int& minimum_baseline,
                       int& natural_baseline) const;

    /**
     * @brief Allocates size for the widget.
     *
     * @param width the allocated width.
     * @param height the allocated height.
     * @param baseline the allocated baseline.
     */
    void size_allocate_vfunc(int width, int height, int baseline);

    /**
     * @brief cleanup method. This method is always called by the base class
     * destructor. Override this method to unparent the widgets connected to
     * this
     */
    virtual void cleanup();
};
}  // namespace ui
