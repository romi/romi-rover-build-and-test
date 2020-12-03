cmake_minimum_required(VERSION 2.6)
project(camera_recorder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(camera_recorder camera_recorder.c camera_recorder_main.c)
target_link_libraries(camera_recorder librcom romi r)
install(TARGETS camera_recorder DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/camera_recorder_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/camera_recorder_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
