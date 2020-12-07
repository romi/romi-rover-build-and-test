
cmake_minimum_required(VERSION 3.10)

set(SRCS_PARSER
  src/tests_main.cpp
  src/test_parser.cpp
  ../BrushMotorController/Parser.cpp
  ../BrushMotorController/Parser.h)

add_executable(romi_rover_unit_tests_parser ${SRCS_PARSER})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../BrushMotorController" )

target_link_libraries(romi_rover_unit_tests_parser gtest gmock)

add_test(
    NAME romi_rover_unit_tests_parser
    COMMAND romi_rover_unit_tests_parser
)

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_unit_tests_parser_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES romi_rover_unit_tests_parser)
endif()

