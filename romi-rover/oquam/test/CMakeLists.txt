
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )


set(SRCS_SECTION
  src/tests_main.cpp
  src/tests_section.cpp
  ../src/Section.h
  ../src/Section.cpp
  ../src/v.h
  ../src/v.c
  )

add_executable(romi_rover_unit_tests_oquam_section ${SRCS_SECTION})
target_link_libraries(romi_rover_unit_tests_oquam_section rpp gtest gmock)



set(SRCS_OQUAMSTEPPER
  src/tests_main.cpp
  src/tests_oquamstepper.cpp
  ../src/ICNCController.h
  ../src/StepperController.h
  ../src/StepperController.cpp
  ../src/Oquam.h
  ../src/Oquam.cpp
  ../src/v.h
  ../src/v.c
  ../src/plotter.cpp
  ../src/plotter.h
  ../src/Section.h
  ../src/Section.cpp
  ../src/Script.h
  ../src/Script.cpp
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
target_link_libraries(romi_rover_unit_tests_oquam_stepper rpp romirover romi gtest gmock)



if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_oquam_coverage
            EXECUTABLE ctest -V ${n_cores}
            EXCLUDE "src/main.cpp" 
            DEPENDENCIES romi_rover_unit_tests_oquam_section
                         )
endif()

