set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/com.moura.Progress.svg)
set(APP_DESKTOP_ENTRY ${CMAKE_CURRENT_SOURCE_DIR}/Progress.desktop)

include(GNUInstallDirs)
install(FILES ${APP_DESKTOP_ENTRY}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/)
install(FILES ${APP_ICON}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/progress-tracker/)