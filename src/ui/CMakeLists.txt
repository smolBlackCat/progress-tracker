find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

add_library(ui window.cpp
               application.cpp
               create_board_dialog.cpp
               board-card-button.cpp)

# Link GTKMM libraries
target_link_libraries(ui PRIVATE ${GTKMM_LIBRARIES})
target_include_directories(ui PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(ui PRIVATE ${GTKMM_LIBRARY_DIRS})
target_compile_options(ui PRIVATE ${GTKMM_CFLAGS_OTHER})

target_link_libraries(ui PRIVATE ${TINYXML2_LIB})