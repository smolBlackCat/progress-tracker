#include "exceptions.h"

board_parse_error::board_parse_error(const std::string& what_arg)
    : std::invalid_argument(what_arg) {

}