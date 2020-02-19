cmake_minimum_required(VERSION 2.6)
project(logger)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(logger logger.c logger_main.c)
target_link_libraries(logger rcom romi r)
install(TARGETS logger DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/logger_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/logger_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
