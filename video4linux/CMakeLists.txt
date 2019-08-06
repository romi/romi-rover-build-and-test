cmake_minimum_required(VERSION 2.6)
project(video4linux)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(video4linux video4linux.c video4linux_main.c camera_v4l.c)
target_link_libraries(video4linux rcom jpeg)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_SOURCE_DIR}/video4linux_main.c
  COMMAND rcgen code ${CMAKE_SOURCE_DIR}/video4linux_main.c ${CMAKE_SOURCE_DIR}/rcgen.json
  DEPENDS ${CMAKE_SOURCE_DIR}/rcgen.json
  )
