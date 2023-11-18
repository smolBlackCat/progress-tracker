add_executable(core_test core.cpp)
add_executable(board-card-button-test board-card-button.cpp)

# TODO: Using the system path is not the best practise.
target_link_libraries(core_test PRIVATE core)
target_link_libraries(core_test PRIVATE ${TINYXML2_LIB})
target_include_directories(core_test PRIVATE ${PROJECT_SOURCE_DIR}/src/core)

target_link_libraries(core_test PRIVATE Catch2::Catch2WithMain)

target_link_libraries(board-card-button-test PRIVATE core)
target_include_directories(board-card-button-test PRIVATE ${PROJECT_SOURCE_DIR}/src/core)
target_link_libraries(board-card-button-test PRIVATE ui)
target_include_directories(board-card-button-test PRIVATE ${PROJECT_SOURCE_DIR}/src/ui)
target_link_libraries(board-card-button-test PRIVATE ${GTKMM_LIBRARIES})
target_compile_options(board-card-button-test PRIVATE ${GTKMM_CFLAGS_OTHER})
target_include_directories(board-card-button-test PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(board-card-button-test PRIVATE ${GTKMM_LIBRARY_DIRS})
