
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )


set(SRCS_SPEEDCONVERTER 
  src/tests_main.cpp
  src/tests_speedconverter.cpp
  mock/mock_navigation.h
  ../include/SpeedConverter.h
  ../src/SpeedConverter.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_speedconverter ${SRCS_SPEEDCONVERTER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_speedconverter rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_speedconverter
    COMMAND romi_rover_unit_tests_userinterface_speedconverter
)


set(SRCS_SPEEDCONTROLLER 
  src/tests_main.cpp
  src/tests_speedcontroller.cpp
  mock/mock_navigation.h
  ../include/DefaultSpeedController.h
  ../src/DefaultSpeedController.cpp
  ../include/SpeedConverter.h
  ../src/SpeedConverter.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_speedcontroller ${SRCS_SPEEDCONTROLLER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_speedcontroller rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_speedcontroller
    COMMAND romi_rover_unit_tests_userinterface_speedcontroller
)


set(SRCS_JOYSTICK 
  src/tests_main.cpp
  src/tests_joystick.cpp
  mock/Linux-Mock.cpp
  ../include/Joystick.h
  ../include/LinuxJoystick.h
  ../src/LinuxJoystick.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_joystick ${SRCS_JOYSTICK})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_joystick rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_joystick
    COMMAND romi_rover_unit_tests_userinterface_joystick
)


set(SRCS_JOYSTICKINPUTDEVICE 
  src/tests_main.cpp
  src/tests_joystickinputdevice.cpp
  mock/mock_eventmapper.h
  mock/mock_joystick.h
  ../include/JoystickInputDevice.h
  ../src/JoystickInputDevice.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_joystickinputdevice ${SRCS_JOYSTICKINPUTDEVICE})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_joystickinputdevice rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_joystickinputdevice
    COMMAND romi_rover_unit_tests_userinterface_joystickinputdevice
)


set(SRCS_UIFACTORY 
  src/tests_main.cpp
  src/tests_uifactory.cpp
  ../include/UIOptions.h
  ../include/UIFactory.h
  ../src/UIFactory.cpp
  ../include/FakeDisplay.h
  ../include/FakeNavigation.h
  ../include/RemoteNavigation.h
  ../src/RemoteNavigation.cpp
  ../include/CrystalDisplay.h
  ../src/CrystalDisplay.cpp
  ../include/UIEventMapper.h
  ../src/UIEventMapper.cpp
  ../include/LinuxJoystick.h
  ../src/LinuxJoystick.cpp
  ../include/JoystickInputDevice.h
  ../src/JoystickInputDevice.cpp
 )

add_executable(romi_rover_unit_tests_userinterface_uifactory ${SRCS_UIFACTORY})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_uifactory romirover romiserial rcompp rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_uifactory
    COMMAND romi_rover_unit_tests_userinterface_uifactory
)


set(SRCS_RPCNAVIGATIONADAPTER 
  src/tests_main.cpp
  src/tests_remotenavigation.cpp
  ../include/RemoteNavigation.h
  ../src/RemoteNavigation.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_remotenavigation ${SRCS_RPCNAVIGATIONADAPTER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_remotenavigation rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_remotenavigation
    COMMAND romi_rover_unit_tests_userinterface_remotenavigation
)


set(SRCS_CRYSTALDISPLAY 
  src/tests_main.cpp
  src/tests_crystaldisplay.cpp
  mock/mock_romiserialclient.h
  ../include/CrystalDisplay.h
  ../src/CrystalDisplay.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_crystaldisplay ${SRCS_CRYSTALDISPLAY})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_crystaldisplay rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_crystaldisplay
    COMMAND romi_rover_unit_tests_userinterface_crystaldisplay
)


set(SRCS_STATEMACHINE 
  src/tests_main.cpp
  src/tests_statemachine.cpp
  ../include/UserInterface.h
  ../include/StateMachine.h
  )

add_executable(romi_rover_unit_tests_userinterface_statemachine ${SRCS_STATEMACHINE})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_statemachine rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_statemachine
    COMMAND romi_rover_unit_tests_userinterface_statemachine
)


#set(SRCS_USERINTERFACE 
#  src/tests_main.cpp
#  src/tests_userinterface.cpp
#  mock/mock_inputdevice.h
#  mock/mock_navigation.h
#  mock/mock_display.h
#  ../include/UserInterface.h
#  ../src/UserInterface.cpp
#  ../include/StateMachine.h
#  ../include/UIStateTransitions.h
#  ../src/UIStateTransitions.cpp
#  )
#
#add_executable(romi_rover_unit_tests_userinterface_userinterface ${SRCS_USERINTERFACE})
#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
#target_link_libraries(romi_rover_unit_tests_userinterface_userinterface rcompp romirover gtest gmock)
#
#add_test(
#    NAME romi_rover_unit_tests_userinterface_userinterface
#    COMMAND romi_rover_unit_tests_userinterface_userinterface
#)


if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_userinterface_coverage
            EXECUTABLE ctest -V ${n_cores}
            EXCLUDE "src/main.cpp" "src/FakeDisplay.h" "src/FakeNavigation.h" "src/FakeInputDevice.h"
            DEPENDENCIES romi_rover_unit_tests_userinterface_speedconverter
                         romi_rover_unit_tests_userinterface_speedcontroller
                         romi_rover_unit_tests_userinterface_joystick
                         romi_rover_unit_tests_userinterface_joystickinputdevice
                         romi_rover_unit_tests_userinterface_uifactory
                         romi_rover_unit_tests_userinterface_remotenavigation
                         romi_rover_unit_tests_userinterface_crystaldisplay
                         romi_rover_unit_tests_userinterface_statemachine
                         #romi_rover_unit_tests_userinterface_userinterface
                         )
endif()

