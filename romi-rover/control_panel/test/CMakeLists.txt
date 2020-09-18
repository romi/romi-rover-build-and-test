
cmake_minimum_required(VERSION 3.10)

set(SRCS_BUTTONPANEL
  src/tests_main.cpp
  src/test_buttonpanel.cpp
  mock/mock_arduino.h
  ../ControlPanelLcdKeypad/ButtonPanel.h)

set(SRCS_STATE_MACHINE
  src/tests_main.cpp
  src/test_statemachine.cpp
  ../ControlPanelLcdKeypad/StateMachine.cpp
  ../ControlPanelLcdKeypad/StateMachine.h)

set(SRCS_CONTROLPANEL
  src/tests_main.cpp
  src/test_controlpanel.cpp
  mock/mock_arduino.h
  mock/mock_serial.h
  mock/mock_display.h
  mock/mock_relay.h
  ../ControlPanelLcdKeypad/Buttons.h
  ../ControlPanelLcdKeypad/Events.h
  ../ControlPanelLcdKeypad/ButtonPanel.h
  ../ControlPanelLcdKeypad/ControlPanel.h
  ../ControlPanelLcdKeypad/ControlPanel.cpp
  ../ControlPanelLcdKeypad/Menu.h
  ../ControlPanelLcdKeypad/States.h
  ../ControlPanelLcdKeypad/States.cpp
  ../ControlPanelLcdKeypad/Parser.cpp
  ../ControlPanelLcdKeypad/Parser.h
  ../ControlPanelLcdKeypad/StateMachine.cpp
  ../ControlPanelLcdKeypad/StateMachine.h
  ../ControlPanelLcdKeypad/StateMachineTimer.cpp
  ../ControlPanelLcdKeypad/StateMachineTimer.h
  ../ControlPanelLcdKeypad/StateTransition.h
  ../ControlPanelLcdKeypad/StateTransition.cpp
  ../ControlPanelLcdKeypad/ControlPanelTransitions.h
  ../ControlPanelLcdKeypad/ControlPanelTransitions.cpp
  )

add_executable(r_unit_tests_buttonpanel ${SRCS_BUTTONPANEL})
add_executable(r_unit_tests_statemachine ${SRCS_STATE_MACHINE})
add_executable(r_unit_tests_controlpanel ${SRCS_CONTROLPANEL})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../ControlPanelLcdKeypad" )

target_link_libraries(r_unit_tests_buttonpanel gtest gmock)
target_link_libraries(r_unit_tests_statemachine gtest gmock)
target_link_libraries(r_unit_tests_controlpanel gtest gmock)

add_test(
    NAME r_unit_tests_buttonpanel
    COMMAND r_unit_tests_buttonpanel
)

add_test(
    NAME r_unit_tests_statemachine
    COMMAND r_unit_tests_statemachine
)

add_test(
    NAME r_unit_tests_controlpanel
    COMMAND r_unit_tests_controlpanel
)

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_unit_tests_statemachine_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests_statemachine)
endif()

