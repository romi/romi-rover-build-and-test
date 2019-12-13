cmake_minimum_required(VERSION 2.6)
project(fsdb)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(fsdb fsdb_node.c fsdb_main.c)
target_link_libraries(fsdb rcom romi r)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/fsdb_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/fsdb_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
