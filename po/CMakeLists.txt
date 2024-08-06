find_program(MSGMERGE msgmerge REQUIRED)
find_program(MSGFMT msgfmt REQUIRED)

include(GNUInstallDirs)

set(POT_FILENAME ${CMAKE_CURRENT_SOURCE_DIR}/progress-tracker.pot)
set(MO_FILENAME progress-tracker.mo)

set(LOCALE_DIRECTORY ${PROJECT_BINARY_DIR}/locales)
file(MAKE_DIRECTORY ${LOCALE_DIRECTORY})

list(APPEND PO_NAMES bg es en_GB en_US it nl pt_BR ru_RU tr uk_UA)

foreach(PO_NAME IN LISTS PO_NAMES)
	list(APPEND PO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${PO_NAME}.po)
endforeach()

# Updates all the translation files before building the application
foreach(PO_SOURCE PO_NAME IN ZIP_LISTS PO_SOURCES PO_NAMES)
    add_custom_target("update-${PO_NAME}" DEPENDS ${PO_SOURCE})
    add_custom_command(
        TARGET "update-${PO_NAME}" PRE_BUILD
        COMMAND ${MSGMERGE} -U -N ${PO_SOURCE} ${POT_FILENAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(progress-tracker "update-${PO_NAME}")
endforeach()

# Compiles all the translation files onto LOCALE_DIRECTORY directory
foreach(PO_SOURCE PO_NAME IN ZIP_LISTS PO_SOURCES PO_NAMES)
    file(MAKE_DIRECTORY ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/)
    add_custom_command(
        OUTPUT ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME}
        COMMAND ${MSGFMT} --output-file=${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME}
                      ${PO_SOURCE}
        DEPENDS ${PO_SOURCE}
    )
    add_custom_target("compile-${PO_NAME}"
                      DEPENDS ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME})
    add_dependencies(progress-tracker "compile-${PO_NAME}")
endforeach()

foreach(PO_NAME IN LISTS PO_NAMES)
        install(FILES ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME}
                DESTINATION ${LOCALE_FOLDER}${PO_NAME}/LC_MESSAGES)
endforeach()
