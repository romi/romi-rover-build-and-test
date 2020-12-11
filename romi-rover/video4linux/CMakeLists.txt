cmake_minimum_required(VERSION 2.6)
project(video4linux)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(video4linux video4linux.c video4linux_main.c camera_v4l.c)
target_link_libraries(video4linux romi rcom r jpeg)
install(TARGETS video4linux DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/video4linux_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/video4linux_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
