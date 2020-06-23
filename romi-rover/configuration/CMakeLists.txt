cmake_minimum_required(VERSION 2.6)
project(configuration)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(configuration configuration.c configuration_main.c)
target_link_libraries(configuration rcom r)
install(TARGETS configuration DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/configuration_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/configuration_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
