cmake_minimum_required(VERSION 2.6)
project(user-interface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../arduino_libraries/RomiSerial" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

set(SOURCES
  include/DefaultSpeedController.h
  include/DefaultEventTimer.h
  include/Display.h
  include/EventsAndStates.h
  include/EventSource.h
  include/EventTimer.h
  include/FakeDisplay.h
  include/FakeInputDevice.h
  include/FakeNavigation.h
  include/FakeNotifications.h
  include/FluidSoundNotifications.h
  include/InputDevice.h
  include/Joystick.h
  include/JoystickEventMapper.h
  include/JoystickInputDevice.h
  include/RoverScriptEngine.h
  include/FakeDisplay.h
  include/CrystalDisplay.h
  include/LinuxJoystick.h
  include/Menu.h
  include/Notifications.h
  include/RemoteNavigation.h
  include/RemoteWeeder.h
  include/ScriptEngine.h
  include/Rover.h
  include/ScriptMenu.h
  include/ScriptList.h
  include/SpeedController.h
  include/SpeedConverter.h
  include/StateMachine.h
  include/UIEventMapper.h
  include/UIFactory.h
  include/UIOptions.h 
  include/UIStateMachine.h
  include/UserInterface.h
  src/DefaultSpeedController.cpp
  src/DefaultEventTimer.cpp
  src/FluidSoundNotifications.cpp
  src/RoverScriptEngine.cpp
  src/JoystickInputDevice.cpp
  src/CrystalDisplay.cpp
  src/LinuxJoystick.cpp
  src/main.cpp
  src/RemoteNavigation.cpp
  src/RemoteWeeder.cpp
  src/Rover.cpp
  src/ScriptMenu.cpp
  src/ScriptList.cpp
  src/SpeedConverter.cpp
  src/UIEventMapper.cpp
  src/UIFactory.cpp
  src/UIOptions.cpp
  src/UIStateMachine.cpp
  src/UserInterface.cpp
  )

add_executable(user-interface ${SOURCES})
target_link_libraries(user-interface romirover rcompp rpp romiserial fluidsynth)


if(BUILD_TESTS)
    add_subdirectory(test)
endif()

install(TARGETS user-interface DESTINATION bin/romi)
