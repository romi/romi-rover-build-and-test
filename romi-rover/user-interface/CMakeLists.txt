cmake_minimum_required(VERSION 2.6)
project(user-interface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../arduino_libraries/RomiSerial" )

set(SOURCES
  src/DefaultSpeedController.h
  src/DefaultSpeedController.cpp
  src/Display.h
  src/EventsAndStates.h
  src/FakeDisplay.h
  src/FakeNavigation.h
  src/InputDevice.h
  src/Joystick.h
  src/JoystickEventMapper.h
  src/JoystickInputDevice.h
  src/JoystickInputDevice.cpp
  src/FakeDisplay.h
  src/CrystalDisplay.h
  src/CrystalDisplay.cpp
  src/FakeWeeder.h
  src/LinuxJoystick.h
  src/LinuxJoystick.cpp
  src/main.cpp
  src/RemoteNavigation.h
  src/RemoteNavigation.cpp
  src/SpeedController.h
  src/SpeedConverter.h
  src/SpeedConverter.cpp
  src/StateMachine.h
  src/UIEventMapper.h
  src/UIEventMapper.cpp
  src/UIFactory.h
  src/UIFactory.cpp
  src/UIOptions.h
  src/UIOptions.cpp
  src/UIStateTransitions.cpp
  src/UserInterface.h
  src/UserInterface.cpp
  )

add_executable(user-interface ${SOURCES})
target_link_libraries(user-interface romirover rcompp rpp romiserial)


if(BUILD_TESTS)
    add_subdirectory(test)
endif()

install(TARGETS user-interface DESTINATION bin/romi)
