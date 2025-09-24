cmake_minimum_required(VERSION 3.10)

project(Progress VERSION 1.7.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# General Settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(DEVELOPMENT "Build the application in development mode" ON)
option(FLATPAK "Defines whether the application is a flatpak distribution" OFF)
option(BUILD_APP "Build the the application's facilities (e.g. window, widgets, and dialog)" ON)

if (DEVELOPMENT)
    set(CMAKE_BUILD_TYPE "Debug")
    set(APPLICATION_ID "io.github.smolblackcat.Progress.Devel")
    set(EXECUTABLE_NAME "progress-debug")
else()
    set(CMAKE_BUILD_TYPE "Release")
    set(APPLICATION_ID "io.github.smolblackcat.Progress")
    set(EXECUTABLE_NAME "progress")
endif()

set(APP_NAME "Progress")
set(APP_ICON ${APPLICATION_ID}.svg)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)
find_program(GLIB_COMPILE_SCHEMAS NAMES glib-compile-schemas REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(ADWAITA REQUIRED libadwaita-1)
pkg_check_modules(SIGCXX REQUIRED sigc++-3.0)

# Crossuid dependency
add_subdirectory(subprojects/crossguid)

find_package(Intl REQUIRED)
find_package(spdlog REQUIRED)
find_package(Catch2 3)

find_library(TINYXML2 libtinyxml2 tinyxml2 REQUIRED)

if (LINUX)
    include(GNUInstallDirs)
    set(LOCALE_FOLDER "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/locale/")
elseif (WIN32)
    set(LOCALE_FOLDER "./locales/")
endif()

add_subdirectory(src)

if (BUILD_APP)
    add_subdirectory(po)
    add_subdirectory(data)
endif()

if(Catch2_FOUND AND DEVELOPMENT)
    add_subdirectory(test)
    include(CTest)
    add_test(NAME Board COMMAND test/board-test)
    add_test(NAME CardList COMMAND test/cardlist-test)
    add_test(NAME Card COMMAND test/card-test)
    add_test(NAME TestColorable COMMAND test/colorable-test)
endif()

set(CPACK_PACKAGE_CHECKSUM SHA512)

if (LINUX)
    set(CPACK_GENERATOR "DEB" "TXZ")
    set(CPACK_STRIP_FILES YES)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)
    set(
        CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Gabriel de Moura <smolblackcat@proton.me>")
    set(CPACK_DEBIAN_PACKAGE_NAME "progress-tracker")
    set(CPACK_DEBIAN_FILE_NAME "progress-tracker-${Progress_VERSION_MAJOR}.${Progress_VERSION_MINOR}.deb")
    set(CPACK_DEBIAN_PACKAGE_VERSION "${Progress_VERSION_MAJOR}.${Progress_VERSION_MINOR}.${Progress_VERSION_PATCH}")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple app for kanban todo lists")
    set(CPACK_DEBIAN_PACKAGE_SECTION "gnome")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/smolBlackCat/progress-tracker")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS64" "ZIP")

    if (DEVELOPMENT)
        set(APP_WINDOWS_ICON io.github.smolblackcat.Progress.Devel.ico)
    else()
        set(APP_WINDOWS_ICON io.github.smolblackcat.Progress.ico)
    endif()

    include(InstallRequiredSystemLibraries)
    set(CPACK_PACKAGE_NAME "Progress")
    set(CPACK_PACKAGE_VENDOR "smolblackcat")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple app for kanban todo lists")
    set(CPACK_PACKAGE_VERSION "${Progress_VERSION_MAJOR}.${Progress_VERSION_MINOR}.${Progress_VERSION_PATCH}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Progress")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

    # NSIS specific options
    set(CPACK_NSIS_DISPLAY_NAME "Progress")
    set(CPACK_NSIS_PACKAGE_NAME "Progress")
    set(CPACK_NSIS_CONTACT "smolblackcat@proton.me")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "WriteRegStr HKLM 'Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Uninstall\\\\Progress' 'DisplayIcon' '$INSTDIR\\\\${APP_WINDOWS_ICON}'")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "\
  CreateShortCut '$SMPROGRAMS\\\\Progress.lnk' '$INSTDIR\\\\progress-tracker.exe' '' '$INSTDIR\\\\${APP_WINDOWS_ICON}' 0 '' '' 'Progress' SW_SHOWNORMAL
")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$SMPROGRAMS\\\\Progress.lnk'")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/data/io.github.smolblackcat.Progress.ico")
    set(CPACK_NSIS_HELP_LINK "https://github.com/smolBlackCat/progress-tracker/issues")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/smolBlackCat/progress-tracker")
endif()

include(CPack)
