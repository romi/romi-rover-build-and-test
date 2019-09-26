cmake_minimum_required(VERSION 2.6)
project(picamera)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(picamera picamera.cc picamera_main.c)
target_link_libraries(picamera rcom romi r raspicam)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/picamera_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/picamera_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
