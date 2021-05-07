cmake_minimum_required(VERSION 3.10)
project(test-hardware)


if((CMAKE_SYSTEM_PROCESSOR MATCHES ".*arm") OR (CMAKE_SYSTEM_PROCESSOR MATCHES ".*aarch64"))
    set(PICAMERA_LIBRARY_PATH /opt/vc/lib)
    link_directories(${PICAMERA_LIBRARY_PATH})
endif()


add_executable(04-test-joystick 04-test-joystick.cpp)
target_link_libraries(04-test-joystick romi)
install(TARGETS 04-test-joystick DESTINATION bin)

#

add_executable(05-tests-serial-1-read 05-tests-serial/test1-read.cpp)
target_link_libraries(05-tests-serial-1-read romi r)
install(TARGETS 05-tests-serial-1-read DESTINATION bin)

add_executable(05-tests-serial-1-write 05-tests-serial/test1-write.cpp)
target_link_libraries(05-tests-serial-1-write romi r)
install(TARGETS 05-tests-serial-1-write DESTINATION bin)


#

if((CMAKE_SYSTEM_PROCESSOR MATCHES ".*arm") OR (CMAKE_SYSTEM_PROCESSOR MATCHES ".*aarch64"))
  add_executable(06-tests-camera-grab 06-tests-camera/test-grab.cpp)
  target_link_libraries(06-tests-camera-grab romi r)
  install(TARGETS 06-tests-camera-grab DESTINATION bin)
endif()

add_executable(06-tests-remote-camera-grab 06-tests-camera/test-remote-grab.cpp)
target_link_libraries(06-tests-remote-camera-grab romi r)
install(TARGETS 06-tests-remote-camera-grab DESTINATION bin)

#

add_executable(99-python-pca 99-python-pca/pca.cpp)
target_link_libraries(99-python-pca romi)
install(TARGETS 99-python-pca DESTINATION bin)


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
target_link_libraries(07-tests-gpio-power-relay romi r)
install(TARGETS 07-tests-gpio-power-relay DESTINATION bin)

add_executable(07-tests-gpio-security-button 07-tests-gpio/test-security-button.cpp ${GPIO_SOURCES})
target_link_libraries(07-tests-gpio-security-button romi r)
install(TARGETS 07-tests-gpio-security-button DESTINATION bin)
