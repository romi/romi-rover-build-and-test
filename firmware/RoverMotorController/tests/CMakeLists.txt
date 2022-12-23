cmake_minimum_required(VERSION 3.10)

set(SRCS
  src/tests_main.cpp
  src/picontroller_tests.cpp
  src/speedenvelope_tests.cpp
  src/motorcontroller_tests.cpp
  src/motorcontrollercommands_tests.cpp
  mocks/mock_arduino.h
  mocks/mock_pwm.h
  mocks/mock_encoder.h
  ../MotorController.cpp
  ../MotorControllerCommands.cpp
  )

add_executable(libromi_unit_tests_motorcontroller ${SRCS})

target_link_libraries(libromi_unit_tests_motorcontroller romiserial
                        gtest
                        gmock)

target_include_directories(libromi_unit_tests_motorcontroller
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

add_test(
        NAME libromi_unit_tests_motorcontroller
        COMMAND libromi_unit_tests_motorcontroller
        )

if (BUILD_COVERAGE)
        SETUP_TARGET_FOR_COVERAGE_LCOV(
                            NAME libromi_unit_tests_motorcontroller_coverage
                            EXECUTABLE ctest -V ${n_cores} -R "libromi"
                            #EXCLUDE "src/rcom-registry.cpp" "include/*" "examples/*"
                            DEPENDENCIES libromi_unit_tests_motorcontroller
                            )
endif()
