cmake_minimum_required(VERSION 2.6)
project(wheel_odometry)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_SOURCE_DIR}/wheel_odometry_main.c
  COMMAND rcgen code ${CMAKE_SOURCE_DIR}/wheel_odometry_main.c ${CMAKE_SOURCE_DIR}/rcgen.json
  DEPENDS ${CMAKE_SOURCE_DIR}/rcgen.json
  )

add_executable(wheel_odometry wheel_odometry.c wheel_odometry_main.c)
target_link_libraries(wheel_odometry rcom romi m)
