
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )


set(SRCS_SPEEDCONTROLLER 
  src/tests_main.cpp
  src/tests_speedcontroller.cpp
  mock/mock_navigation.h
  ../src/SpeedController.h
  ../src/SpeedController.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_speedcontroller ${SRCS_SPEEDCONTROLLER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_speedcontroller r gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_speedcontroller
    COMMAND romi_rover_unit_tests_userinterface_speedcontroller
)


set(SRCS_RPCNAVIGATIONADAPTER 
  src/tests_main.cpp
  src/tests_rpcnavigationclient.cpp
  mock/mock_rpcclient.h
  ../src/RPCNavigationClientAdaptor.h
  ../src/RPCNavigationClientAdaptor.cpp
  )

add_executable(romi_rover_unit_tests_userinterface_rpcnavigationclientadaptor ${SRCS_RPCNAVIGATIONADAPTER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_userinterface_rpcnavigationclientadaptor r gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_rpcnavigationclientadaptor
    COMMAND romi_rover_unit_tests_userinterface_rpcnavigationclientadaptor
)


if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_userinterface_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES romi_rover_unit_tests_userinterface_speedcontroller romi_rover_unit_tests_userinterface_rpcnavigationclientadaptor)
endif()

