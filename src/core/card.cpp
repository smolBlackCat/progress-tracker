#include "card.h"

Card::Card(const std::string& name, const Gdk::RGBA& color)
    : Item{name}, color{color} {}

void Card::set_color(const Gdk::RGBA& color) {
    this->color = color;
    modified = true;
}

Gdk::RGBA Card::get_color() const { return color; }

bool Card::is_color_set() { return color != NO_COLOR; }
