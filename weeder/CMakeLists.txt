cmake_minimum_required(VERSION 2.6)
project(weeder)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(weeder weeder.c weeder_main.c weeding.c)
target_link_libraries(weeder rcom romi m)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_SOURCE_DIR}/weeder_main.c
  COMMAND rcgen code ${CMAKE_SOURCE_DIR}/weeder_main.c ${CMAKE_SOURCE_DIR}/rcgen.json
  DEPENDS ${CMAKE_SOURCE_DIR}/rcgen.json
  )
