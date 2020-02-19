cmake_minimum_required(VERSION 2.6)
project(grbl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(grbl grbl.c grbl_main.c)
target_link_libraries(grbl rcom romi r)
install(TARGETS grbl DESTINATION romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/grbl_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/grbl_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
