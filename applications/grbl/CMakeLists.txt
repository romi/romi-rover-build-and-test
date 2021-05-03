cmake_minimum_required(VERSION 3.10)
project(grbl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(grbl grbl.c grbl_main.c)
target_link_libraries(grbl rcom romi r)
install(TARGETS grbl DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/grbl_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/grbl_main.c rcgen.json
  DEPENDS rcgen.json
  )
