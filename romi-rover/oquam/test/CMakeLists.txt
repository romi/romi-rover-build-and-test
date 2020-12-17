
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )

set(SRCS_OQUAMSTEPPER
  src/tests_main.cpp
  src/tests_oquamstepper.cpp
  ../src/ICNCController.h
  ../src/StepperController.h
  ../src/StepperController.cpp
  ../src/Oquam.h
  ../src/Oquam.cpp
  ../src/CNCRange.h
  ../src/CNCRange.cpp
  ../src/v.h
  ../src/v.c
  ../src/plotter.c
  ../src/plotter.h
  ../src/script.h
  ../src/script.c
  ../src/IFileCabinet.h
  ../src/IFolder.h
  ../src/DebugWeedingSession.h
  ../src/DebugWeedingSession.cpp
  ../src/Oquam.cpp
  ../../arduino_libraries/RomiSerial/IInputStream.h
  ../../arduino_libraries/RomiSerial/IOutputStream.h
  ../../arduino_libraries/RomiSerial/RSerial.h
  ../../arduino_libraries/RomiSerial/RSerial.cpp
  ../../arduino_libraries/RomiSerial/CRC8.h
  ../../arduino_libraries/RomiSerial/EnvelopeParser.h
  ../../arduino_libraries/RomiSerial/EnvelopeParser.cpp
  ../../arduino_libraries/RomiSerial/RomiSerialClient.h
  ../../arduino_libraries/RomiSerial/RomiSerialClient.cpp
  )

add_executable(romi_rover_unit_tests_oquam_stepper ${SRCS_OQUAMSTEPPER})
target_link_libraries(romi_rover_unit_tests_oquam_stepper r romi gtest gmock)

