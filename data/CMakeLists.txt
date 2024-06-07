set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/io.github.smolblackcat.Progress.svg)
set(APP_WINDOWS_ICON ${CMAKE_CURRENT_SOURCE_DIR}/io.github.smolblackcat.Progress.ico)
set(APP_DESKTOP_ENTRY ${CMAKE_CURRENT_SOURCE_DIR}/io.github.smolblackcat.Progress.desktop)
set(APP_METAINFO ${CMAKE_CURRENT_SOURCE_DIR}/io.github.smolblackcat.Progress.metainfo.xml)

if(${FLATPAK})
    install(FILES ${APP_DESKTOP_ENTRY}
            DESTINATION /app/share/applications/)
    install(FILES ${APP_ICON}
            DESTINATION /app/share/icons/hicolor/scalable/apps/)
    install(FILES ${APP_METAINFO}
            DESTINATION /app/share/metainfo/)
elseif(${WINDOWS})
        install(FILES ${APP_WINDOWS_ICON}
                DESTINATION ${PROJECT_BINARY_DIR}/Progress)
else()
    install(FILES ${APP_DESKTOP_ENTRY}
            DESTINATION /usr/share/applications/)
    install(FILES ${APP_ICON}
            DESTINATION /usr/share/icons/hicolor/scalable/apps/)
    install(FILES ${APP_METAINFO}
            DESTINATION /usr/share/metainfo/)
endif()