file(GLOB_RECURSE CORE_SOURCES "../src/core/*.cpp" "../src/core/*.h")

add_executable(cardlist-test cardlist-test.cpp ${CORE_SOURCES})
add_executable(board-test board-test.cpp ${CORE_SOURCES})
add_executable(card-test card-test.cpp ${CORE_SOURCES})
add_executable(modification-test modification-test.cpp ${CORE_SOURCES})
add_executable(colorable-test colorable-test.cpp ${CORE_SOURCES})
add_executable(gen-heavy-board heavy-board.cpp ${CORE_SOURCES})

list(APPEND TEST_EXECUTABLES
    board-test
    cardlist-test
    card-test
    modification-test
    colorable-test
    gen-heavy-board)

foreach(EXECUTABLE ${TEST_EXECUTABLES})
    target_link_libraries(${EXECUTABLE} PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(${EXECUTABLE} PRIVATE ${TINYXML2})
    target_link_libraries(${EXECUTABLE} PRIVATE spdlog::spdlog)

    target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_BINARY_DIR}/src/)
    target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/src/)
    target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/src/core)
endforeach()
