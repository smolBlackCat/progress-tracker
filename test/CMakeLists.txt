add_executable(cardlist-test cardlist-test.cpp)
add_executable(board-test board-test.cpp)
add_executable(card-test card-test.cpp)
add_executable(modification-test modification-test.cpp)
add_executable(colorable-test colorable-test.cpp)

list(APPEND TEST_EXECUTABLES
    board-test
    cardlist-test
    card-test
    modification-test
    colorable-test)

foreach(EXECUTABLE ${TEST_EXECUTABLES})
    target_link_libraries(${EXECUTABLE} PRIVATE libcore)
    target_link_libraries(${EXECUTABLE} PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(${EXECUTABLE} PRIVATE ${TINYXML2})

    target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_BINARY_DIR}/src/)
    target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/src/)
endforeach()
