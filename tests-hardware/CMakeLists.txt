cmake_minimum_required(VERSION 2.6)
project(test-hardware)

set(SOURCES_TEST_JOYSTICK
  04-test-joystick.cpp
  )

add_executable(04-test-joystick ${SOURCES_TEST_JOYSTICK})
target_link_libraries(04-test-joystick romi)

install(TARGETS 04-test-joystick DESTINATION bin)
