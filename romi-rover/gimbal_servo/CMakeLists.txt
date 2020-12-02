cmake_minimum_required(VERSION 2.6)
project(gimbal_servo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(gimbal_servo gimbal_servo.c gimbal_servo_main.c)
target_link_libraries(gimbal_servo romi rcomlib r m)
install(TARGETS gimbal_servo DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/gimbal_servo_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/gimbal_servo_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
