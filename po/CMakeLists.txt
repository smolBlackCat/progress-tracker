find_program(MSGMERGE msgmerge REQUIRED)
find_program(MSGFMT msgfmt REQUIRED)

set(POT_FILENAME ${CMAKE_CURRENT_SOURCE_DIR}/progress-tracker.pot)
set(MO_FILENAME progress-tracker.mo)

set(LOCALE_DIRECTORY ${PROJECT_BINARY_DIR}/locales)
file(MAKE_DIRECTORY ${LOCALE_DIRECTORY})
list(APPEND LINGUAS
        bg
        es
        en_GB
        en_US
        it
        nl
        pl
        pt_BR
        ru_RU
        tr
        uk_UA)

foreach(PO_NAME ${LINGUAS})
    # Updates all the translation files before building the application whenver the
    # progress-tracker.pot is updated
    add_custom_target("update-${PO_NAME}" DEPENDS "${PO_NAME}.po")
    add_custom_command(
        TARGET "update-${PO_NAME}" PRE_BUILD
        COMMAND ${MSGMERGE} -U -N "${PO_NAME}.po" ${POT_FILENAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(progress-tracker "update-${PO_NAME}")

    # Compiles all the translation files onto LOCALE_DIRECTORY directory
    file(MAKE_DIRECTORY ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/)
    add_custom_command(
    OUTPUT ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME}
    COMMAND ${MSGFMT} --output-file=${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME}
                  "${PO_NAME}.po"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS "${PO_NAME}.po")
    add_custom_target("compile-${PO_NAME}"
                  DEPENDS ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME})
    add_dependencies(progress-tracker "compile-${PO_NAME}")

    # Install the compiled translation files
    if (WIN32)
        install(FILES ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME}
            DESTINATION ${LOCALE_FOLDER}${PO_NAME}/LC_MESSAGES)
    elseif(LINUX)
        install(FILES ${LOCALE_DIRECTORY}/${PO_NAME}/LC_MESSAGES/${MO_FILENAME}
            DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/${PO_NAME}/LC_MESSAGES)
    endif()
endforeach()