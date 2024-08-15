file(GLOB_RECURSE CORE_SOURCES "../src/core/*.cpp" "../src/core/*.h")

add_executable(cardlist-test cardlist-test.cpp ${CORE_SOURCES})
add_executable(board-test board-test.cpp ${CORE_SOURCES})
add_executable(card-test card-test.cpp ${CORE_SOURCES})
add_executable(modification-test modification-test.cpp ${CORE_SOURCES})

list(APPEND TEST_EXECUTABLES
    board-test
    cardlist-test
    card-test
    modification-test)

foreach(EXECUTABLE ${TEST_EXECUTABLES})
    target_link_libraries(${EXECUTABLE} PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(${EXECUTABLE} PRIVATE ${GTKMM_LIBRARIES})
    target_link_libraries(${EXECUTABLE} PRIVATE ${TINYXML2})

    target_include_directories(${EXECUTABLE} PUBLIC ${GTKMM_INCLUDE_DIRS})
    target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/src/core)

    target_compile_options(${EXECUTABLE} PUBLIC ${GTKMM_CFLAGS_OTHER})
endforeach()
