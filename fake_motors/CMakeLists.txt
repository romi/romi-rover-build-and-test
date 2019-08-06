cmake_minimum_required(VERSION 2.6)
project(fake_motors)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(fake_motors fake_motors.c fake_motors_main.c)
target_link_libraries(fake_motors rcom m)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_SOURCE_DIR}/fake_motors_main.c
  COMMAND rcgen code ${CMAKE_SOURCE_DIR}/fake_motors_main.c ${CMAKE_SOURCE_DIR}/rcgen.json
  DEPENDS ${CMAKE_SOURCE_DIR}/rcgen.json
  )
