cmake_minimum_required(VERSION 2.6)
project(joystick_navigation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(joystick_navigation joystick_navigation.c joystick_navigation_main.c)
target_link_libraries(joystick_navigation romi rcom r)
install(TARGETS joystick_navigation DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/joystick_navigation_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/joystick_navigation_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
