
cmake_minimum_required(VERSION 3.10)

set(SRCS_STATE_MACHINE
  src/tests_main.cpp
  src/test_statemachine.cpp
  ../ControlPanelLcdKeypad/StateMachine.cpp
  ../ControlPanelLcdKeypad/StateMachine.h)

add_executable(r_unit_tests_statemachine ${SRCS_STATE_MACHINE})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../ControlPanelLcdKeypad" )

target_link_libraries(r_unit_tests_statemachine gtest)

add_test(
    NAME r_unit_tests_statemachine
    COMMAND r_unit_tests_statemachine
)

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_unit_tests_statemachine_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests_statemachine)
endif()

