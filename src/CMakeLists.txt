configure_file(app_info.h.in app_info.h)

file(GLOB_RECURSE CORE_SOURCES "core/*.h" "core/*.cpp")
file(GLOB_RECURSE WIDGET_SOURCES "widgets/*.cpp" "widgets/*.h")
file(GLOB_RECURSE DIALOG_SOURCES "dialog/*.cpp" "dialog/*.h")

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
    ${WIDGET_SOURCES}
    ${DIALOG_SOURCES}
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp)

if ((WIN32 AND (NOT DEVELOPMENT)))
    target_link_options(progress-tracker PRIVATE "-mwindows")
endif()

target_link_libraries(progress-tracker PUBLIC ${GTKMM_LIBRARIES})
target_link_libraries(progress-tracker PUBLIC ${TINYXML2})
target_link_libraries(progress-tracker PUBLIC ${ADWAITA_LIBRARIES})
target_link_libraries(progress-tracker PUBLIC ${INTL_LIBRARIES})
target_link_libraries(progress-tracker PUBLIC ${APP_LIBS})

target_link_directories(progress-tracker PUBLIC ${GTKMM_LIBRARY_DIR})

target_include_directories(progress-tracker PUBLIC ${TINYXML2_INCLUDE_DIRS})
target_include_directories(progress-tracker PUBLIC ${GTKMM_INCLUDE_DIRS})
target_include_directories(progress-tracker PUBLIC ${ADWAITA_INCLUDE_DIRS})
target_include_directories(progress-tracker PUBLIC ${INTL_INCLUDE_DIRS})
target_include_directories(progress-tracker PUBLIC ${APP_INCLUDES})

target_compile_options(progress-tracker PRIVATE ${GTKMM_CFLAGS_OTHER})

# Resources Settings
set(GLIB_FLAGS --target=resources.cpp)
set(GLIB_FLAGS ${GLIB_FLAGS} --generate-source)
set(GLIB_FLAGS ${GLIB_FLAGS} --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/resources/ui)

file(GLOB_RECURSE RESOURCES "resources/ui/*.ui" "resources/ui/*.css")

add_custom_command(
    OUTPUT resources.cpp
    COMMAND ${GLIB_COMPILE_RESOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources.xml ${GLIB_FLAGS}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources.xml
    ${RESOURCES}
)
add_custom_target(progress-resources DEPENDS resources.cpp)
add_dependencies(progress-tracker progress-resources)

if(WIN32)
    find_program(PYTHON NAMES python3 python)
    add_custom_command(TARGET progress-tracker POST_BUILD
                       COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/build-aux/fetch-win-deps.py
                       WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
    add_custom_command(TARGET progress-tracker POST_BUILD
                       COMMAND cp ${CMAKE_SOURCE_DIR}/data/io.github.smolblackcat.Progress.svg share/icons/
                       WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
    add_custom_command(TARGET progress-tracker POST_BUILD
                       COMMAND cp ${CMAKE_SOURCE_DIR}/data/io.github.smolblackcat.Progress.Devel.svg share/icons/
                       WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

    install(TARGETS progress-tracker
    		RUNTIME
    		DESTINATION ".")
    install(PROGRAMS ${PROJECT_BINARY_DIR}/gdbus.exe
            DESTINATION ".")
    install(DIRECTORY ${PROJECT_BINARY_DIR}/DLLS/
    		DESTINATION "."
            FILES_MATCHING PATTERN "*.dll")
    install(DIRECTORY ${PROJECT_BINARY_DIR}/share/
    		DESTINATION share)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/lib/
    		DESTINATION lib)
else()
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/progress-tracker
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            RENAME ${EXECUTABLE_NAME})
endif()
