cmake_minimum_required(VERSION 3.10)
project(test-hardware)

add_executable(03-tests-navigation-distance 03-tests-navigation/navigation-distance.cpp)
target_link_libraries(03-tests-navigation-distance romi)

#

add_executable(03-tests-steering 03-tests-steering/test-steering.cpp)
target_link_libraries(03-tests-steering romi)

add_executable(03-tests-steering-0 03-tests-steering/test-steering-0.cpp)
target_link_libraries(03-tests-steering-0 romi)

#

add_executable(04-test-joystick 04-test-joystick.cpp)
target_link_libraries(04-test-joystick romi)
install(TARGETS 04-test-joystick DESTINATION bin)

#

add_executable(05-tests-serial-1-read 05-tests-serial/test1-read.cpp)
target_link_libraries(05-tests-serial-1-read romi)
install(TARGETS 05-tests-serial-1-read DESTINATION bin)

add_executable(05-tests-serial-1-write 05-tests-serial/test1-write.cpp)
target_link_libraries(05-tests-serial-1-write romi)
install(TARGETS 05-tests-serial-1-write DESTINATION bin)


#

if(PI_BUILD)
  add_executable(06-tests-camera-grab 06-tests-camera/test-grab.cpp)
  target_link_libraries(06-tests-camera-grab romi)
  install(TARGETS 06-tests-camera-grab DESTINATION bin)
endif()

add_executable(06-tests-remote-camera-grab 06-tests-camera/test-remote-grab.cpp)
target_link_libraries(06-tests-remote-camera-grab romi)
install(TARGETS 06-tests-remote-camera-grab DESTINATION bin)

#


add_executable(10-calibration-move-and-grab 10-calibration/move-and-grab.cpp)
target_link_libraries(10-calibration-move-and-grab romi)

#

set(GPIO_SOURCES
  07-tests-gpio/GpioFileDescriptor.cpp
  07-tests-gpio/GpioFileDescriptor.h
  07-tests-gpio/GpioChip.cpp
  07-tests-gpio/GpioChip.h
  07-tests-gpio/GpioInput.cpp
  07-tests-gpio/GpioInput.h
  07-tests-gpio/GpioOutput.cpp
  07-tests-gpio/GpioOutput.h
  07-tests-gpio/IGpio.h
  07-tests-gpio/RpiGpio.cpp
  07-tests-gpio/RpiGpio.h
)

add_executable(07-tests-gpio-power-relay 07-tests-gpio/test-power-relay.cpp ${GPIO_SOURCES})
target_link_libraries(07-tests-gpio-power-relay romi)
install(TARGETS 07-tests-gpio-power-relay DESTINATION bin)

add_executable(07-tests-gpio-security-button 07-tests-gpio/test-security-button.cpp ${GPIO_SOURCES})
target_link_libraries(07-tests-gpio-security-button romi)
install(TARGETS 07-tests-gpio-security-button DESTINATION bin)
