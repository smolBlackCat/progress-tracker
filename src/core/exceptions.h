#include <stdexcept>

class board_parse_error : public std::invalid_argument {
public:
    board_parse_error(const std::string& what_arg);
};