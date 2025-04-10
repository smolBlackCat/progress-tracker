#pragma once

/**
 * @brief Modifiable is a behaviour class for objects that register the
 * modified state
 */
class Modifiable {
public:
    Modifiable(bool modified = false);
    virtual ~Modifiable() = default;

    /**
     * @brief Produces true if a modification was made to the object.
     */
    virtual bool get_modified() const;

    /**
     * @brief Sets the modified state of the object.
     */
    virtual void set_modified(bool modified = true);

protected:
    bool modified = false;
};