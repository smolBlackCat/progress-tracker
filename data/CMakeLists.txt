set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/com.moura.Progress.svg)
set(APP_DESKTOP_ENTRY ${CMAKE_CURRENT_SOURCE_DIR}/Progress.desktop)

include(GNUInstallDirs)
install(FILES ${APP_DESKTOP_ENTRY}
        DESTINATION /usr/share/applications/)
install(FILES ${APP_ICON}
        DESTINATION /usr/share/progress-tracker/)