set(APP_WINDOWS_ICON io.github.smolblackcat.Progress.ico)

configure_file(io.github.smolblackcat.Progress.desktop.in ${APPLICATION_ID}.desktop)
set(APP_DESKTOP_ENTRY ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.desktop)

configure_file(io.github.smolblackcat.Progress.metainfo.xml.in ${APPLICATION_ID}.metainfo.xml)
set(APP_METAINFO ${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_ID}.metainfo.xml)

set(SYMBOLIC_ICON io.github.smolblackcat.Progress-symbolic.svg)

if (LINUX AND NOT FLATPAK)
    install(FILES ${APP_DESKTOP_ENTRY}
            DESTINATION /usr/share/applications/)
    install(FILES ${APP_METAINFO}
            DESTINATION /usr/share/metainfo/)
    install(FILES ${APP_ICON}
            DESTINATION /usr/share/icons/hicolor/scalable/apps/)
    install(FILES ${SYMBOLIC_ICON}
            DESTINATION /usr/share/icons/hicolor/symbolic/apps/
            RENAME ${APPLICATION_ID}-symbolic.svg)
elseif(${FLATPAK})
    install(FILES ${APP_DESKTOP_ENTRY}
            DESTINATION /app/share/applications/)
    install(FILES ${APP_ICON}
            DESTINATION /app/share/icons/hicolor/scalable/apps/)
    install(FILES ${APP_METAINFO}
            DESTINATION /app/share/metainfo/)
    install(FILES ${SYMBOLIC_ICON}
            DESTINATION /app/share/icons/hicolor/symbolic/apps/
            RENAME ${APPLICATION_ID}-symbolic.svg)
endif()