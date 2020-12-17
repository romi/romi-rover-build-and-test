cmake_minimum_required(VERSION 2.6)
project(joystick_navigation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
  DebugMotorController.h
  EventMapper.h
  IEventMapper.h
  IJoystick.h
  IMotorController.h
  INavigation.h
  IStateMachine.h
  IStateTransition.h
  Joystick.cpp
  Joystick.h
  JoystickStateTransitions.h
  main.cpp
  Navigation.h
  StateMachine.h
  StateTransition.h
  )

add_executable(joystick_navigation ${SOURCES})

target_link_libraries(joystick_navigation romi rcom)
install(TARGETS joystick_navigation DESTINATION bin/romi)
