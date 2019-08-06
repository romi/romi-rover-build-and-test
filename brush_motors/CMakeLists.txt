cmake_minimum_required(VERSION 2.6)
project(brush_motors)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(brush_motors brush_motors.c brush_motors_main.c)
target_link_libraries(brush_motors rcom m)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/brush_motors_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/brush_motors_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
