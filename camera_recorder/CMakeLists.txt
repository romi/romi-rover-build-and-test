cmake_minimum_required(VERSION 2.6)
project(camera_recorder)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(camera_recorder camera_recorder.c camera_recorder_main.c)
target_link_libraries(camera_recorder rcom romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/camera_recorder_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/camera_recorder_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
