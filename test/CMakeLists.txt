file(GLOB_RECURSE CORE_SOURCES "../src/core/*.cpp" "../src/core/*.h")

add_executable(core_test core.cpp ${CORE_SOURCES})

target_link_libraries(core_test PRIVATE Catch2::Catch2WithMain)

target_link_libraries(core_test PUBLIC ${GTKMM_LIBRARIES})
target_include_directories(core_test PUBLIC ${GTKMM_INCLUDE_DIRS})
target_include_directories(core_test PRIVATE ${PROJECT_SOURCE_DIR}/src/core)
target_compile_options(core_test PUBLIC ${GTKMM_CFLAGS_OTHER})
target_link_libraries(core_test PRIVATE ${TINYXML2_LIB})
