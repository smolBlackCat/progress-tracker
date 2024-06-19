file(GLOB_RECURSE PROGRESS_DIALOG_SOURCES
    "*.cpp")

add_library(progress-dialog ${PROGRESS_DIALOG_SOURCES})

target_link_libraries(progress-dialog PUBLIC ${GTKMM_LIBRARIES})

target_include_directories(progress-dialog PUBLIC ${GTKMM_INCLUDE_DIRS})
target_include_directories(progress-dialog PUBLIC ${PROJECT_BINARY_DIR}/src)