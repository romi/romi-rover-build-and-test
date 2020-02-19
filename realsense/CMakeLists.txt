cmake_minimum_required(VERSION 2.6)
project(realsense)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(realsense realsense.cc realsense_main.c convert.c)
target_link_libraries(realsense rcom romi r jpeg png realsense2)
install(TARGETS realsense DESTINATION romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/realsense_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/realsense_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
