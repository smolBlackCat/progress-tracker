find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

add_library(ui window.cpp create_board_dialog.cpp)

# Link GTKMM libraries
target_link_libraries(ui PRIVATE ${GTKMM_LIBRARIES})
target_include_directories(ui PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(ui PRIVATE ${GTKMM_LIBRARY_DIRS})
target_compile_options(ui PRIVATE ${GTKMM_CFLAGS_OTHER})

if (TESTS_ENABLED)
    add_executable(dialog_test dialog_tests.cpp)
    target_link_libraries(dialog_test PRIVATE ui)
    target_include_directories(dialog_test PRIVATE /usr/include/catch2/)
    target_include_directories(dialog_test PRIVATE ${APP_INCLUDES})
    target_link_libraries(dialog_test PRIVATE ${GTKMM_LIBRARIES})
    target_include_directories(dialog_test PRIVATE ${GTKMM_INCLUDE_DIRS})
    target_link_directories(dialog_test PRIVATE ${GTKMM_LIBRARY_DIRS})
    target_compile_options(dialog_test PRIVATE ${GTKMM_CFLAGS_OTHER})
endif()