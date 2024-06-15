cmake_minimum_required(VERSION 3.10)
project(ProgressTracker VERSION 1.4.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# General Settings
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(APP_NAME "Progress (Debug)")
    set(APPLICATION_ID "io.github.smolblackcat.Progress.Devel")
    set(APP_ICON io.github.smolblackcat.Progress.Devel.svg)
    set(EXECUTABLE_NAME "progress-debug")
    set(DEBUG true)
else()
    set(APP_NAME "Progress")
    set(APPLICATION_ID "io.github.smolblackcat.Progress")
    set(APP_ICON io.github.smolblackcat.Progress.svg)
    set(EXECUTABLE_NAME "progress")
endif()

# GTK setup
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(ADWAITA REQUIRED libadwaita-1)

find_package(Intl REQUIRED)
find_package(Catch2 3)

# Tiny XML setup
find_library(TINYXML2_LIB libtinyxml2 tinyxml2 REQUIRED)

add_subdirectory(src)
add_subdirectory(po)
add_subdirectory(data)

if(${Catch2_FOUND})
    add_subdirectory(test)
    include(CTest)
    add_test(NAME CoreTest COMMAND test/core_test)
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Gabriel de Moura")
set(CPACK_DEBIAN_PACKAGE_NAME "progress-tracker")
set(CPACK_DEBIAN_FILE_NAME "progress-tracker-${ProgressTracker_VERSION_MAJOR}.${ProgressTracker_VERSION_MINOR}.deb")
set(CPACK_DEBIAN_PACKAGE_VERSION "${ProgressTracker_VERSION_MAJOR}.${ProgressTracker_VERSION_MINOR}")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple app for kanban todo lists")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtkmm-4.0-0, libadwaita-1-dev, libtinyxml2-10")
set(CPACK_DEBIAN_PACKAGE_SECTION "gnome")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/smolBlackCat/progress-tracker")
include(CPack)