set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/io.github.progresstracker.Progress.svg)
set(APP_DESKTOP_ENTRY ${CMAKE_CURRENT_SOURCE_DIR}/io.github.progresstracker.Progress.desktop)
set(APP_METAINFO ${CMAKE_CURRENT_SOURCE_DIR}/io.github.progresstracker.Progress.metainfo.xml)

if(${FLATPAK})
    install(FILES ${APP_DESKTOP_ENTRY}
            DESTINATION /app/share/applications/)
    install(FILES ${APP_ICON}
            DESTINATION /app/share/icons/hicolor/scalable/apps/)
    install(FILES ${APP_METAINFO}
            DESTINATION /app/share/metainfo/)
            
else()
    install(FILES ${APP_DESKTOP_ENTRY}
            DESTINATION /usr/share/applications/)
    install(FILES ${APP_ICON}
            DESTINATION /usr/share/icons/hicolor/scalable/apps/)
    install(FILES ${APP_METAINFO}
            DESTINATION /usr/share/metainfo/)
endif()