#include "modifiable.h"

Modifiable::Modifiable(bool modified) : modified(modified) {}

bool Modifiable::get_modified() const { return modified; }

void Modifiable::set_modified(bool modified) { this->modified = modified; }
