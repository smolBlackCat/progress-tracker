if (DEVELOPMENT)
    set(APP_WINDOWS_ICON io.github.smolblackcat.Progress.Devel.ico)
else()
    set(APP_WINDOWS_ICON io.github.smolblackcat.Progress.ico)
endif()

configure_file(io.github.smolblackcat.Progress.desktop.in ${APPLICATION_ID}.desktop)
set(APP_DESKTOP_ENTRY ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.desktop)

configure_file(io.github.smolblackcat.Progress.metainfo.xml.in ${APPLICATION_ID}.metainfo.xml)
set(APP_METAINFO ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.metainfo.xml)

set(SYMBOLIC_ICON io.github.smolblackcat.Progress-symbolic.svg)

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