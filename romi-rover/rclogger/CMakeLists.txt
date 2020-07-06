cmake_minimum_required(VERSION 2.6)
project(logger)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(rclogger logger.c rclogger_main.c)
target_link_libraries(rclogger rcom romi r)
install(TARGETS rclogger DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/rclogger_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/rclogger_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
