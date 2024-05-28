if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(APPLICATION_ID "io.github.smolblackcat.Progress")
else()
    set(DEBUG True)
    set(APPLICATION_ID "io.github.smolblackcat.ProgressDebug")
endif()

configure_file(app_info.h.in app_info.h)

# Set Libraries and include directories
add_subdirectory(ui)
add_subdirectory(core)
list(APPEND APP_LIBS ui)
list(APPEND APP_LIBS core)
list(APPEND APP_INCLUDES "${PROJECT_SOURCE_DIR}/ui")
list(APPEND APP_INCLUDES "${PROJECT_SOURCE_DIR}/core")

add_executable(progress-tracker main.cpp ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp)

# Setup GTK
if ((WINDOWS AND CMAKE_BUILD_TYPE STREQUAL "Release"))
    target_link_options(progress-tracker PRIVATE "-mwindows")
endif()
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

# Resources Settings
set(GLIB_FLAGS --target=resources.cpp)
set(GLIB_FLAGS ${GLIB_FLAGS} --generate-source)
set(GLIB_FLAGS ${GLIB_FLAGS} --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/resources/ui)

list(APPEND RESOURCES resources/ui/io.github.smolblackcat.Progress.svg
                      resources/ui/app-window.ui
                      resources/ui/create-board-dialog.ui
                      resources/ui/style.css
                      resources/ui/style-dark.css)

add_custom_command(
    OUTPUT resources.cpp
    COMMAND glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/resources.xml ${GLIB_FLAGS}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources.xml
    ${RESOURCES}
)
add_custom_target(generate_resources DEPENDS resources.cpp)
add_dependencies(progress-tracker generate_resources)

if (${FLATPAK})
    install(TARGETS progress-tracker
            RUNTIME
            DESTINATION /app/bin/)
elseif(${WINDOWS})
    # Add code that packages app into an installer
else()
    install(TARGETS progress-tracker
            RUNTIME
            DESTINATION /usr/bin/)
endif()