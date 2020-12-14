
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )

set(SRCS_ROVERCONFIGURATION 
  src/tests_main.cpp
  src/tests_roverconfiguration.cpp
  ../src/RoverConfiguration.h
  )

add_executable(r_unit_tests_navigation_roverconfiguration ${SRCS_ROVERCONFIGURATION})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(r_unit_tests_navigation_roverconfiguration r gtest gmock)

add_test(
    NAME r_unit_tests_navigation_roverconfiguration
    COMMAND r_unit_tests_navigation_roverconfiguration
)



set(SRCS_MOTORCONTROLLER 
  src/tests_main.cpp
  src/tests_brushmotordriver.cpp
  ../src/IMotorDriver.h
  ../src/BrushMotorDriver.h
  ../../arduino_libraries/RomiSerial/MessageParser.h
  ../../arduino_libraries/RomiSerial/MessageParser.cpp
  ../../arduino_libraries/RomiSerial/EnvelopeParser.h
  ../../arduino_libraries/RomiSerial/EnvelopeParser.cpp
  ../../arduino_libraries/RomiSerial/RomiSerialClient.h
  ../../arduino_libraries/RomiSerial/RomiSerialClient.cpp
  )

add_executable(r_unit_tests_navigation_motorcontroller ${SRCS_MOTORCONTROLLER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(r_unit_tests_navigation_motorcontroller r rcom gtest gmock)

add_test(
    NAME r_unit_tests_navigation_motorcontroller
    COMMAND r_unit_tests_navigation_motorcontroller
)




set(SRCS_WHEELODOMETRY 
  src/tests_main.cpp
  src/tests_wheelodometry.cpp
  ../src/WheelOdometry.h
  ../src/WheelOdometry.cpp
  )

add_executable(r_unit_tests_navigation_wheelodometry ${SRCS_WHEELODOMETRY})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(r_unit_tests_navigation_wheelodometry r gtest gmock)

add_test(
    NAME r_unit_tests_navigation_wheelodometry
    COMMAND r_unit_tests_navigation_wheelodometry
)



set(SRCS_NAVIGATION 
  src/tests_main.cpp
  src/tests_navigation.cpp
  mock/mock_motordriver.h
  ../src/INavigation.h
  ../src/Navigation.h
  ../src/Navigation.cpp
  ../src/WheelOdometry.h
  ../src/WheelOdometry.cpp
  ../src/IMotorDriver.h
  )

add_executable(r_unit_tests_navigation_navigation ${SRCS_NAVIGATION})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(r_unit_tests_navigation_navigation r gtest gmock)

add_test(
    NAME r_unit_tests_navigation_navigation
    COMMAND r_unit_tests_navigation_navigation
)



set(SRCS_RPCNAVIGATION 
  src/tests_main.cpp
  src/tests_rpcnavigation.cpp
  mock/mock_navigation.h
  ../src/INavigation.h
  ../src/RPCNavigation.h
  ../src/RPCNavigation.cpp
  )

add_executable(r_unit_tests_navigation_rpcnavigation ${SRCS_RPCNAVIGATION})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(r_unit_tests_navigation_rpcnavigation r rcom gtest gmock)

add_test(
    NAME r_unit_tests_navigation_rpcnavigation
    COMMAND r_unit_tests_navigation_rpcnavigation
)



set(SRCS_CONFIGURATION 
  src/tests_main.cpp
  src/tests_configuration.cpp
  ../src/JSONConfiguration.h
  ../src/ConfigurationFile.h
  )

add_executable(r_unit_tests_navigation_configuration ${SRCS_CONFIGURATION})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(r_unit_tests_navigation_configuration r rcom gtest gmock)

add_test(
    NAME r_unit_tests_navigation_configuration
    COMMAND r_unit_tests_navigation_configuration
)



if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_navigation_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests_navigation_motorcontroller r_unit_tests_navigation_roverconfiguration r_unit_tests_navigation_wheelodometry  r_unit_tests_navigation_navigation r_unit_tests_navigation_rpcnavigation r_unit_tests_navigation_configuration)
endif()

