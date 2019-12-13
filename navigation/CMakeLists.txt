cmake_minimum_required(VERSION 2.6)
project(navigation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(navigation navigation.c navigation_main.c)
target_link_libraries(navigation romi rcom r)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/navigation_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/navigation_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
