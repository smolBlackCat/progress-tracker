add_library(core item.cpp card.cpp cardlist.cpp board.cpp colorable.h exceptions.cpp)

target_link_libraries(core PRIVATE tinyxml2)
target_link_libraries(core PUBLIC ${GTKMM_LIBRARIES})
target_compile_options(core PUBLIC ${GTKM_CFLAGS_OTHER})

target_include_directories(core PUBLIC ${GTKMM_INCLUDE_DIRS})
target_include_directories(core PRIVATE ${PROJECT_BINARY_DIR}/src)