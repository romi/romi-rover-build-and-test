cmake_minimum_required(VERSION 2.6)
project(control_panel)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(control_panel control_panel.c control_panel_main.c)
target_link_libraries(control_panel rcom r m)
install(TARGETS control_panel DESTINATION romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/control_panel_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/control_panel_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
