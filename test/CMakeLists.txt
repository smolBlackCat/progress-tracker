add_executable(core_test core.cpp)

target_link_libraries(core_test PRIVATE core)
target_link_libraries(core_test PRIVATE Catch2::Catch2WithMain)
target_link_libraries(core_test PRIVATE ${TINYXML2_LIB})
target_include_directories(core_test PRIVATE ${PROJECT_SOURCE_DIR}/src/core)
