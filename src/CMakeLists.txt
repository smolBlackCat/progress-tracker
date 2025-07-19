configure_file(app_info.h.in app_info.h)

file(GLOB_RECURSE CORE_SOURCES "core/*.cpp")

add_library(libcore STATIC ${CORE_SOURCES})
target_link_libraries(libcore PRIVATE ${TINYXML2})
target_link_libraries(libcore PRIVATE crossguid)
target_link_libraries(libcore PUBLIC ${SIGCXX_LIBRARIES})
target_link_libraries(libcore PUBLIC spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)

target_link_directories(libcore PUBLIC ${SIGCXX_LIBRARY_DIRS})

target_include_directories(libcore PRIVATE ${SIGCXX_INCLUDE_DIRS})
target_include_directories(libcore PRIVATE ${TINYXML2_INCLUDE_DIRS})
target_include_directories(libcore
    PUBLIC ${CMAKE_SOURCE_DIR}/subprojects/crossguid/include/crossguid/)
target_include_directories(libcore PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_compile_options(libcore PUBLIC ${SIGCXX_CFLAGS})

if (BUILD_APP)
    file(GLOB_RECURSE WIDGET_SOURCES "widgets/*.cpp" "widgets/*.h")
    file(GLOB_RECURSE DIALOG_SOURCES "dialog/*.cpp" "dialog/*.h")
    add_executable(progress-tracker
    main.cpp
    app-context.h
    app-context.cpp
    application.cpp
    application.h
    window.cpp
    window.h
    utils.h
    utils.cpp
    ${WIDGET_SOURCES}
    ${DIALOG_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp)

    if ((WIN32 AND (NOT DEVELOPMENT)))
        target_link_options(progress-tracker PRIVATE "-mwindows")
    endif()

    target_link_libraries(progress-tracker
    PRIVATE ${GTKMM_LIBRARIES}
    PRIVATE ${ADWAITA_LIBRARIES}
    PRIVATE ${Intl_LIBRARIES}
    PRIVATE spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>
    PRIVATE libcore)

    target_link_directories(progress-tracker PRIVATE ${GTKMM_LIBRARY_DIR})

    target_include_directories(progress-tracker
    PRIVATE ${GTKMM_INCLUDE_DIRS}
    PRIVATE ${ADWAITA_INCLUDE_DIRS}
    PRIVATE ${Intl_INCLUDE_DIRS}
    PRIVATE ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

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
    ${RESOURCES})
    add_custom_target(progress-resources DEPENDS resources.cpp)
    add_dependencies(progress-tracker progress-resources)

    if(WIN32)
        find_program(PYTHON NAMES python3 python)
        add_custom_command(TARGET progress-tracker POST_BUILD
                       COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/build-aux/setup-gtk-environment.py
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
endif()
