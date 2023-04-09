add_library(core item.cpp card.cpp cardlist.cpp board.cpp)

# Link the catch2 header for testing

if (TESTS_ENABLED)
    add_executable(core_test tests.cpp)
    # TODO: Using the system path is not the best practise.
    target_link_libraries(core_test PRIVATE core)
    target_link_libraries(core_test PRIVATE ${TINYXML2_LIB})
    target_include_directories(core_test PRIVATE ${PROJECT_SOURCE_DIR}/core)
    target_include_directories(core_test PRIVATE /usr/include/catch2/)

    # Tests
    add_test(NAME CoreTest COMMAND core_test)
endif()