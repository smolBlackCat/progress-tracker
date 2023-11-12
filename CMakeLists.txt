cmake_minimum_required(VERSION 3.10)
project(ProgressTracker VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# GTK setup
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

# Tiny XML setup
find_library(TINYXML2_LIB libtinyxml2 tinyxml2 REQUIRED)

set(TESTS_ENABLED true)
if (TESTS_ENABLED)
    enable_testing()
endif()

add_subdirectory(src)
add_subdirectory(test)

add_test(NAME CoreTest COMMAND test/core_test)