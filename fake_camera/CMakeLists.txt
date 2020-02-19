cmake_minimum_required(VERSION 2.6)
project(fake_camera)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(fake_camera fake_camera.c fake_camera_main.c)
target_link_libraries(fake_camera rcom r)
install(TARGETS fake_camera DESTINATION romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/fake_camera_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/fake_camera_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
