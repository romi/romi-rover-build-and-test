
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )


set(SRCS_SPEEDCONTROLLER 
  src/tests_main.cpp
  src/tests_speedcontroller.cpp
  mock/mock_navigation.h
  ../src/DefaultSpeedController.h
  ../src/DefaultSpeedController.cpp
  ../src/SpeedConverter.h
  ../src/SpeedConverter.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_speedcontroller ${SRCS_SPEEDCONTROLLER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_speedcontroller rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_speedcontroller
    COMMAND romi_rover_unit_tests_userinterface_speedcontroller
)


set(SRCS_UIFACTORY 
  src/tests_main.cpp
  src/tests_uifactory.cpp
  ../src/UIOptions.h
  ../src/UIFactory.h
  ../src/UIFactory.cpp
  ../src/FakeDisplay.h
  ../src/FakeNavigation.h
  ../src/RemoteNavigation.h
  ../src/RemoteNavigation.cpp
  ../src/CrystalDisplay.h
  ../src/CrystalDisplay.cpp
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
  ../src/RemoteNavigation.h
  ../src/RemoteNavigation.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_remotenavigation ${SRCS_RPCNAVIGATIONADAPTER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_remotenavigation rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_remotenavigation
    COMMAND romi_rover_unit_tests_userinterface_remotenavigation
)


if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_userinterface_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES romi_rover_unit_tests_userinterface_speedcontroller
                         romi_rover_unit_tests_userinterface_uifactory
                         romi_rover_unit_tests_userinterface_remotenavigation)
endif()

