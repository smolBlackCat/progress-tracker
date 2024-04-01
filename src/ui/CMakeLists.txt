add_library(ui window.cpp
               application.cpp
               board-dialog.cpp
               create_board_dialog.cpp
               preferences-board-dialog.cpp
               board-card-button.cpp
               board-widget.cpp
               cardlist-widget.cpp
               card.cpp
               editable-label-header.cpp
               i18n.h)

# Link GTKMM libraries
target_link_libraries(ui PRIVATE ${GTKMM_LIBRARIES})
target_include_directories(ui PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(ui PRIVATE ${GTKMM_LIBRARY_DIRS})
target_compile_options(ui PRIVATE ${GTKMM_CFLAGS_OTHER})

target_link_libraries(ui PRIVATE ${ADWAITA_LIBRARIES})
target_include_directories(ui PRIVATE ${ADWAITA_INCLUDE_DIRS})
target_link_directories(ui PRIVATE ${ADWAITA_LIBRARY_DIRS})
target_compile_options(ui PRIVATE ${ADWAITA_CFLAGS_OTHER})

target_link_libraries(ui PRIVATE ${TINYXML2_LIB})

target_link_libraries(ui PRIVATE ${INTL_LIBRARIES})
target_include_directories(ui PRIVATE ${INTL_INCLUDE_DIR})
target_include_directories(ui PRIVATE ${PROJECT_BINARY_DIR}/src)