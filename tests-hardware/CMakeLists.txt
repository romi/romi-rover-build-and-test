cmake_minimum_required(VERSION 2.6)
project(test-hardware)

add_executable(04-test-joystick 04-test-joystick.cpp)
target_link_libraries(04-test-joystick romi)
install(TARGETS 04-test-joystick DESTINATION bin)

#

add_executable(05-test-serial-camera-1 05-test-serial-camera/test1.cpp)
target_link_libraries(05-test-serial-camera-1 romi r)
install(TARGETS 05-test-serial-camera-1 DESTINATION bin)

#

add_executable(05-test-serial-camera-2-host 05-test-serial-camera/test2-host.cpp)
target_link_libraries(05-test-serial-camera-2-host romi romiserial r)
install(TARGETS 05-test-serial-camera-2-host DESTINATION bin)

add_executable(05-test-serial-camera-2-camera 05-test-serial-camera/test2-camera.cpp)
target_link_libraries(05-test-serial-camera-2-camera romi romiserial r)
install(TARGETS 05-test-serial-camera-2-camera DESTINATION bin)
