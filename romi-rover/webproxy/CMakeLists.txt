cmake_minimum_required(VERSION 3.10)
project(webproxy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(webproxy webproxy.c webproxy_main.c)
target_link_libraries(webproxy rcom r m)
install(TARGETS webproxy DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/webproxy_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/webproxy_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
