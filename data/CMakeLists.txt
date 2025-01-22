set(APP_WINDOWS_ICON ${APPLICATION_ID}.ico)

configure_file(io.github.smolblackcat.Progress.desktop.in.in ${APPLICATION_ID}.desktop.in)
set(APP_DESKTOP_ENTRY ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.desktop)

add_custom_target(i18n-desktop)
add_custom_command(
    TARGET i18n-desktop PRE_BUILD
    COMMAND ${MSGFMT} --desktop --template=${APPLICATION_ID}.desktop.in --keyword=_ -d ${CMAKE_SOURCE_DIR}/po -o ${APPLICATION_ID}.desktop
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(progress-tracker i18n-desktop)

configure_file(io.github.smolblackcat.Progress.metainfo.xml.in.in ${APPLICATION_ID}.metainfo.xml.in)
set(APP_METAINFO ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.metainfo.xml)

add_custom_target(i18n-metainfo)
add_custom_command(
    TARGET i18n-metainfo PRE_BUILD
    COMMAND ${MSGFMT} --xml --template=${APPLICATION_ID}.metainfo.xml.in -d ${CMAKE_SOURCE_DIR}/po -o ${APPLICATION_ID}.metainfo.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(progress-tracker i18n-metainfo)

set(SYMBOLIC_ICON io.github.smolblackcat.Progress-symbolic.svg)
set(GSCHEMA io.github.smolblackcat.Progress.gschema.xml)

if (LINUX)
    install(FILES ${APP_DESKTOP_ENTRY}
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications/)
    install(FILES ${APP_METAINFO}
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo/)
    install(FILES ${APP_ICON}
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps/)
    install(FILES ${SYMBOLIC_ICON}
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/symbolic/apps/
            RENAME ${APPLICATION_ID}-symbolic.svg)
    install(FILES ${GSCHEMA}
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/glib-2.0/schemas/)
elseif(WIN32)
    install(FILES ${APP_DESKTOP_ENTRY}
            DESTINATION share/applications/)
    install(FILES ${APP_METAINFO}
            DESTINATION share/metainfo/)
    install(FILES ${APP_ICON}
            DESTINATION share/icons/hicolor/scalable/apps/)
    install(FILES ${SYMBOLIC_ICON}
            DESTINATION share/icons/hicolor/symbolic/apps/
            RENAME ${APPLICATION_ID}-symbolic.svg)
    install(FILES ${APP_WINDOWS_ICON}
            DESTINATION ".")
endif()
