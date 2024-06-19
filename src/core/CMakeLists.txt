add_library(core item.cpp card.cpp cardlist.cpp board.cpp exceptions.cpp)

target_link_libraries(core PRIVATE tinyxml2)

target_include_directories(core PRIVATE ${PROJECT_BINARY_DIR}/src)