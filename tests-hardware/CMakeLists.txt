cmake_minimum_required(VERSION 2.6)
project(test-hardware)

set(SOURCES_TEST_JOYSTICK
  04-test-joystick.cpp
  )

add_executable(04-test-joystick ${SOURCES_TEST_JOYSTICK})
target_link_libraries(04-test-joystick romi)

install(TARGETS 04-test-joystick DESTINATION bin)




set(SOURCES_TEST_SERIAL_CAMERA
  05-test-serial-camera/main.cpp
  )

add_executable(05-test-serial-camera ${SOURCES_TEST_SERIAL_CAMERA})
target_link_libraries(05-test-serial-camera romi r)

install(TARGETS 05-test-serial-camera DESTINATION bin)
