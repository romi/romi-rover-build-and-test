
cmake_minimum_required(VERSION 3.10)

set(SRCS_PARSER
  src/tests_main.cpp
  src/test_parser.cpp
  ../ControlPanelLcdKeypad/IParser.h
  ../ControlPanelLcdKeypad/Parser.cpp
  ../ControlPanelLcdKeypad/Parser.h)

set(SRCS_BUTTONPANEL
  src/tests_main.cpp
  src/test_buttonpanel.cpp
  mock/mock_arduino.h
  ../ControlPanelLcdKeypad/ButtonPanel.h)

set(SRCS_EVENTTIMER
  src/tests_main.cpp
  src/test_eventtimer.cpp
  ../ControlPanelLcdKeypad/EventTimer.h)

set(SRCS_MENU
  src/tests_main.cpp
  src/test_menu.cpp
  ../ControlPanelLcdKeypad/Menu.h)

set(SRCS_STATE_MACHINE
  src/tests_main.cpp
  src/test_statemachine.cpp
  mock/mock_statetransition.h
  ../ControlPanelLcdKeypad/StateMachine.h)

set(SRCS_CONTROLPANEL
  src/tests_main.cpp
  src/test_controlpanel.cpp
  mock/mock_arduino.h
  mock/mock_inputstream.h
  mock/mock_timer.h
  mock/mock_relay.h
  mock/mock_outputstream.h
  mock/mock_display.h
  ../ControlPanelLcdKeypad/ControlPanel.h
  ../ControlPanelLcdKeypad/ControlPanel.cpp
  ../ControlPanelLcdKeypad/States.h
  ../ControlPanelLcdKeypad/States.cpp
  ../ControlPanelLcdKeypad/Events.h
  ../ControlPanelLcdKeypad/Buttons.h
  ../ControlPanelLcdKeypad/ButtonPanel.h
  ../ControlPanelLcdKeypad/Menu.h
  ../ControlPanelLcdKeypad/Parser.cpp
  ../ControlPanelLcdKeypad/Parser.h
  ../ControlPanelLcdKeypad/StateMachine.h
  ../ControlPanelLcdKeypad/EventTimer.h
  ../ControlPanelLcdKeypad/StateTransition.h
  ../ControlPanelLcdKeypad/ControlPanelTransitions.h
  ../ControlPanelLcdKeypad/ControlPanelTransitions.cpp
  )

add_executable(r_unit_tests_controlpanel_parser ${SRCS_PARSER})
add_executable(r_unit_tests_controlpanel_buttonpanel ${SRCS_BUTTONPANEL})
add_executable(r_unit_tests_controlpanel_eventtimer ${SRCS_EVENTTIMER})
add_executable(r_unit_tests_controlpanel_menu ${SRCS_MENU})
add_executable(r_unit_tests_controlpanel_statemachine ${SRCS_STATE_MACHINE})
add_executable(r_unit_tests_controlpanel_controlpanel ${SRCS_CONTROLPANEL})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../ControlPanelLcdKeypad" )

target_link_libraries(r_unit_tests_controlpanel_parser gtest gmock)
target_link_libraries(r_unit_tests_controlpanel_buttonpanel gtest gmock)
target_link_libraries(r_unit_tests_controlpanel_eventtimer gtest gmock)
target_link_libraries(r_unit_tests_controlpanel_menu gtest gmock)
target_link_libraries(r_unit_tests_controlpanel_statemachine gtest gmock)
target_link_libraries(r_unit_tests_controlpanel_controlpanel gtest gmock)

add_test(
    NAME r_unit_tests_controlpanel_parser
    COMMAND r_unit_tests_controlpanel_parser
)

add_test(
    NAME r_unit_tests_controlpanel_buttonpanel
    COMMAND r_unit_tests_controlpanel_buttonpanel
)

add_test(
    NAME r_unit_tests_controlpanel_eventtimer
    COMMAND r_unit_tests_controlpanel_eventtimer
)

add_test(
    NAME r_unit_tests_controlpanel_menu
    COMMAND r_unit_tests_controlpanel_menu
)

add_test(
    NAME r_unit_tests_controlpanel_statemachine
    COMMAND r_unit_tests_controlpanel_statemachine
)

add_test(
    NAME r_unit_tests_controlpanel_controlpanel
    COMMAND r_unit_tests_controlpanel_controlpanel
)

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME r_unit_tests_statemachine_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES r_unit_tests_statemachine)
endif()

