cmake_minimum_required(VERSION 2.6)
project(gimbal_servo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(gimbal_servo gimbal_servo.c gimbal_servo_main.c)
target_link_libraries(gimbal_servo romi rcom r m)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/gimbal_servo_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/gimbal_servo_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
