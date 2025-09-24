list(APPEND TEST_EXECUTABLES
    board-test
    cardlist-test
    card-test
    colorable-test
    board-manager-test
    stress-test)

foreach(EXECUTABLE ${TEST_EXECUTABLES})
    add_executable(${EXECUTABLE} ${EXECUTABLE}.cpp)
    target_link_libraries(${EXECUTABLE} PRIVATE libcore)
    target_link_libraries(${EXECUTABLE} PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(${EXECUTABLE} PRIVATE ${TINYXML2})

    target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_BINARY_DIR}/src/)
    target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/src/)
endforeach()
