
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )

set(SRCS_OQUAMSTEPPER
  src/tests_main.cpp
  src/tests_oquamstepper.cpp
  ../src/JSON.h
  ../src/IBlockController.h
  ../src/OquamStepper.h
  ../src/OquamStepper.cpp
  ../src/Oquam.h
  ../src/Oquam.cpp
  ../src/v.h
  ../src/v.c
  ../src/planner.h
  ../src/planner.c
  ../src/plotter.c
  ../src/plotter.h
  ../src/script.h
  ../src/script_priv.h
  ../src/script.c
  ../../arduino_libraries/RomiSerial/IInputStream.h
  ../../arduino_libraries/RomiSerial/IOutputStream.h
  ../../arduino_libraries/RomiSerial/RSerial.h
  ../../arduino_libraries/RomiSerial/RSerial.cpp
  ../../arduino_libraries/RomiSerial/CRC8.h
  ../../arduino_libraries/RomiSerial/RomiSerialClient.h
  ../../arduino_libraries/RomiSerial/RomiSerialClient.cpp
  )

add_executable(r_unit_tests_oquam_stepper ${SRCS_OQUAMSTEPPER})
target_link_libraries(r_unit_tests_oquam_stepper r gtest gmock)
