cmake_minimum_required(VERSION 2.6)
project(fake_cnc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(fake_cnc fake_cnc.c fake_cnc_main.c)
target_link_libraries(fake_cnc librcom romi r)
install(TARGETS fake_cnc DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/fake_cnc_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/fake_cnc_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
