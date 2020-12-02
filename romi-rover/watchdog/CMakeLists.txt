cmake_minimum_required(VERSION 2.6)
project(watchdog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(watchdog watchdog.c watchdog_main.c)
target_link_libraries(watchdog rcomlib r m)
install(TARGETS watchdog DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/watchdog_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/watchdog_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
