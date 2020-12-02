cmake_minimum_required(VERSION 2.6)
project(script_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/script_engine_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/script_engine_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )

add_executable(script_engine script_engine.c script_engine_main.c)
target_link_libraries(script_engine romi rcomlib r)
install(TARGETS script_engine DESTINATION bin/romi)
