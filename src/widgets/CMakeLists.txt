file(GLOB_RECURSE PROGRESS_WIDGETS_SOURCES
    "*.cpp")

add_library(progress-widgets ${PROGRESS_WIDGETS_SOURCES})

target_link_libraries(progress-widgets PUBLIC ${GTKMM_LIBRARIES})

target_include_directories(progress-widgets PUBLIC ${GTKMM_INCLUDE_DIRS})
target_include_directories(progress-widgets PUBLIC ${PROJECT_BINARY_DIR}/src)