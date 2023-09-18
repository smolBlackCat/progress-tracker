add_executable(core_test core.cpp)

# TODO: Using the system path is not the best practise.
target_link_libraries(core_test PRIVATE core)
target_link_libraries(core_test PRIVATE ${TINYXML2_LIB})
target_include_directories(core_test PRIVATE ${PROJECT_SOURCE_DIR}/src/core)
target_include_directories(core_test PRIVATE /usr/include/catch2/)