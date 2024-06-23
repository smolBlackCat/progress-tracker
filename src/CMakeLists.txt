configure_file(app_info.h.in app_info.h)

# Set Libraries and include directories
add_subdirectory(core)
add_subdirectory(dialog)
add_subdirectory(widgets)

list(APPEND APP_LIBS
     core
     progress-dialog
     progress-widgets)
list(APPEND APP_INCLUDES
     "${PROJECT_SOURCE_DIR}/src"
     "${PROJECT_BINARY_DIR}/src")

add_executable(progress-tracker
    main.cpp
    application.cpp
    application.h
    window.cpp
    window.h
    utils.h
    utils.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp)

if ((WINDOWS AND CMAKE_BUILD_TYPE STREQUAL "Release"))
    target_link_options(progress-tracker PRIVATE "-mwindows")
endif()

target_link_libraries(progress-tracker PRIVATE ${ADWAITA_LIBRARIES})
target_link_libraries(progress-tracker PRIVATE ${INTL_LIBRARIES})
target_link_libraries(progress-tracker PRIVATE ${APP_LIBS})

target_include_directories(progress-tracker PRIVATE ${ADWAITA_INCLUDE_DIRS})
target_include_directories(progress-tracker PRIVATE ${INTL_INCLUDE_DIR})
target_include_directories(progress-tracker PRIVATE ${APP_INCLUDES})

target_compile_options(progress-tracker PRIVATE ${GTKMM_CFLAGS_OTHER})

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
    COMMAND ${GLIB_COMPILE_RESOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources.xml ${GLIB_FLAGS}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources.xml
    ${RESOURCES}
)
add_custom_target(progress-resources DEPENDS resources.cpp)
add_dependencies(progress-tracker progress-resources)

if (${FLATPAK})
    install(TARGETS progress-tracker
            RUNTIME
            DESTINATION /app/bin/)
elseif(${WINDOWS})
    find_program(PYTHON NAMES python3 python)
    add_custom_command(TARGET progress-tracker POST_BUILD
                       COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/build-aux/fetch-win-deps.py
                       WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
    add_custom_command(TARGET progress-tracker POST_BUILD
                       COMMAND cp ${CMAKE_SOURCE_DIR}/data/io.github.smolblackcat.Progress.svg share/icons/
                       WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

else()
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/progress-tracker
            DESTINATION /usr/bin/
            RENAME ${EXECUTABLE_NAME})
endif()