cmake_minimum_required(VERSION 3.10)
project(test-hardware)

add_executable(04-test-joystick 04-test-joystick.cpp)
target_link_libraries(04-test-joystick romi)
install(TARGETS 04-test-joystick DESTINATION bin)

#

add_executable(05-test-serial-camera-1-read 05-test-serial-camera/test1-read.cpp)
target_link_libraries(05-test-serial-camera-1-read romi r)
install(TARGETS 05-test-serial-camera-1-read DESTINATION bin)

add_executable(05-test-serial-camera-1-write 05-test-serial-camera/test1-write.cpp)
target_link_libraries(05-test-serial-camera-1-write romi r)
install(TARGETS 05-test-serial-camera-1-write DESTINATION bin)

#

add_executable(05-test-serial-camera-2-host 05-test-serial-camera/test2-host.cpp)
target_link_libraries(05-test-serial-camera-2-host romi romiserial r)
install(TARGETS 05-test-serial-camera-2-host DESTINATION bin)

add_executable(05-test-serial-camera-2-camera 05-test-serial-camera/test2-camera.cpp)
target_link_libraries(05-test-serial-camera-2-camera romi romiserial r)
install(TARGETS 05-test-serial-camera-2-camera DESTINATION bin)

#

set(GPIO_SOURCES
  06-test-gpio/GpioFileDescriptor.cpp
  06-test-gpio/GpioFileDescriptor.h
  06-test-gpio/GpioChip.cpp
  06-test-gpio/GpioChip.h
  06-test-gpio/GpioInput.cpp
  06-test-gpio/GpioInput.h
  06-test-gpio/GpioOutput.cpp
  06-test-gpio/GpioOutput.h
  06-test-gpio/IGpio.h
  06-test-gpio/RpiGpio.cpp
  06-test-gpio/RpiGpio.h
)

add_executable(06-test-gpio-power-relay 06-test-gpio/test-power-relay.cpp ${GPIO_SOURCES})
target_link_libraries(06-test-gpio-power-relay romi r)
install(TARGETS 06-test-gpio-power-relay DESTINATION bin)

add_executable(06-test-gpio-security-button 06-test-gpio/test-security-button.cpp ${GPIO_SOURCES})
target_link_libraries(06-test-gpio-security-button romi r)
install(TARGETS 06-test-gpio-security-button DESTINATION bin)
