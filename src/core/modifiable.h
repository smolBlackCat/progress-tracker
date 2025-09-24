#pragma once

/**
 * @brief Modifiable is a behaviour class for objects that register the
 * modified state
 */
class Modifiable {
public:
    virtual ~Modifiable() = default;

    virtual bool modified() const = 0;
    virtual void modify(bool m = true) = 0;
};