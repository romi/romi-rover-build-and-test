cmake_minimum_required(VERSION 2.6)
project(interface)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/interface_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/interface_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )

add_executable(interface interface.c interface_main.c)
target_link_libraries(interface romi rcom)
