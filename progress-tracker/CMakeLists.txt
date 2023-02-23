cmake_minimum_required(VERSION 3.10)
project(ProgressTracker VERSION 1.0)

# GTK setup
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(app_info.h.in app_info.h)

# Set Libraries
add_subdirectory(ui)
list(APPEND APP_LIBS ui)
list(APPEND APP_INCLUDES "${PROJECT_SOURCE_DIR}/ui")

add_executable(progress-tracker main.cpp)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
target_link_libraries(progress-tracker PRIVATE ${GTKMM_LIBRARIES})
target_include_directories(progress-tracker PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(progress-tracker PRIVATE ${GTKMM_LIBRARY_DIRS})
target_compile_options(progress-tracker PRIVATE ${GTKMM_CFLAGS_OTHER})

target_link_libraries(progress-tracker PRIVATE ${APP_LIBS})
target_include_directories(progress-tracker PRIVATE ${PROJECT_BINARY_DIR} ${APP_INCLUDES})

target_include_directories(progress-tracker PUBLIC ${PROJECT_BINARY_DIR})