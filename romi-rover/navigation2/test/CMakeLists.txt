
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )

set(SRCS_MOTORCONTROLLER 
  src/tests_main.cpp
  src/tests_brushmotordriver.cpp
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

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_navigation_motorcontroller_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests_navigation_motorcontroller)
endif()

