#include "card.h"

Card::Card(std::string name) : Item{name} {

}

Card::Card(std::string name, unsigned long long id) : Item{name, id} {
    
}

std::string Card::fetch_xml() const {
    return "\t\t<card name=\"" + name + "\">";
}
