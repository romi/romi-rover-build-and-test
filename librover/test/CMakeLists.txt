cmake_minimum_required(VERSION 3.10)

set(SRCS
  src/tests_main.cpp)

add_executable(rover_unit_tests ${SRCS})

target_link_libraries(rover_unit_tests
                        gtest
                        gmock
                        romimocks
                        rover)

target_include_directories(rover_unit_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

add_test(
    NAME rover_unit_tests
    COMMAND rover_unit_tests
)

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME rover_unit_tests_coverage
            EXECUTABLE ctest -V ${n_cores} -R "rover_unit_"
            EXCLUDE '*wrapper*'
            DEPENDENCIES rover_unit_tests)
endif()
