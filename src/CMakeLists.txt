configure_file(app_info.h.in app_info.h)

# Set Libraries and include directories
add_subdirectory(ui)
add_subdirectory(core)
list(APPEND APP_LIBS ui)
list(APPEND APP_LIBS core)
list(APPEND APP_INCLUDES "${PROJECT_SOURCE_DIR}/ui")
list(APPEND APP_INCLUDES "${PROJECT_SOURCE_DIR}/core")

add_executable(progress-tracker main.cpp ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
target_link_libraries(progress-tracker PRIVATE ${GTKMM_LIBRARIES})
target_include_directories(progress-tracker PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(progress-tracker PRIVATE ${GTKMM_LIBRARY_DIRS})
target_compile_options(progress-tracker PRIVATE ${GTKMM_CFLAGS_OTHER})

# i18n Settings
target_link_libraries(progress-tracker PRIVATE ${INTL_LIBRARIES})
target_include_directories(progress-tracker PRIVATE ${INTL_INCLUDE_DIR})

# Link the application's libraries and headers
target_link_libraries(progress-tracker PRIVATE ${APP_LIBS})
target_include_directories(progress-tracker PRIVATE ${PROJECT_BINARY_DIR}/src ${APP_INCLUDES})

target_compile_options(progress-tracker PRIVATE -g)

# Resources Settings
set(GLIB_FLAGS --target=resources.cpp)
set(GLIB_FLAGS ${GLIB_FLAGS} --generate-source)
set(GLIB_FLAGS ${GLIB_FLAGS} --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/ui)

list(APPEND RESOURCES ui/com.moura.Progress.svg
                      ui/app-window.ui
                      ui/create-board-dialog.ui
                      ui/stylesheet.css)

add_custom_command(
    OUTPUT resources.cpp
    COMMAND glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/ui/resources.xml ${GLIB_FLAGS}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui/resources.xml
    ${RESOURCES}
)
add_custom_target(generate_resources DEPENDS resources.cpp)
add_dependencies(progress-tracker generate_resources)

install(TARGETS progress-tracker
        RUNTIME
        DESTINATION /usr/bin/)