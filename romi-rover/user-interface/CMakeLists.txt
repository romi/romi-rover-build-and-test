cmake_minimum_required(VERSION 2.6)
project(user-interface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
  src/DebugNavigation.h
  src/EventsAndStates.h
  src/EventMapper.h
  src/EventMapper.cpp
  src/IEventMapper.h
  src/IJoystick.h
  src/ISpeedController.h
  src/IStateMachine.h
  src/IStateTransition.h
  src/Joystick.h
  src/Joystick.cpp
  src/JoystickStateTransitions.h
  src/JoystickStateTransitions.cpp
  src/main.cpp
  src/RPCNavigationClientAdaptor.h
  src/RPCNavigationClientAdaptor.cpp
  src/SpeedController.h
  src/SpeedController.cpp
  src/StateMachine.h
  src/StateMachine.cpp
  src/StateTransition.h
  )

add_executable(user-interface ${SOURCES})
target_link_libraries(user-interface romirover rcompp rpp)


if(BUILD_TESTS)
    add_subdirectory(test)
endif()

install(TARGETS user-interface DESTINATION bin/romi)
