cmake_minimum_required(VERSION 3.10)
project(ProgressTracker VERSION 1.0)

option(TESTS_ENABLED true)
if (TESTS_ENABLED)
    enable_testing()
endif()

# GTK setup
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

# Tiny XML setup
find_library(TINYXML2_LIB libtinyxml2 tinyxml2 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(app_info.h.in app_info.h)

# Set Libraries and include directories
add_subdirectory(ui)
add_subdirectory(core)
list(APPEND APP_LIBS ui)
list(APPEND APP_LIBS core)
list(APPEND APP_INCLUDES "${PROJECT_SOURCE_DIR}/ui")
list(APPEND APP_INCLUDES "${PROJECT_SOURCE_DIR}/core")

add_executable(progress-tracker main.cpp)

if (TESTS_ENABLED)
    add_test(NAME CoreTest COMMAND core/core_test)
endif()


# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
target_link_libraries(progress-tracker PRIVATE ${GTKMM_LIBRARIES})
target_include_directories(progress-tracker PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(progress-tracker PRIVATE ${GTKMM_LIBRARY_DIRS})
target_compile_options(progress-tracker PRIVATE ${GTKMM_CFLAGS_OTHER})

# Link the application's libraries and headers
target_link_libraries(progress-tracker PRIVATE ${APP_LIBS})
target_include_directories(progress-tracker PRIVATE ${PROJECT_BINARY_DIR} ${APP_INCLUDES})
target_include_directories(progress-tracker PUBLIC ${PROJECT_BINARY_DIR})