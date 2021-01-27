
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )





set(SRCS_OQUAMFACTORY
  src/tests_main.cpp
  src/tests_oquamfactory.cpp
  ../src/OquamFactory.h
  ../src/OquamFactory.cpp
  )

add_executable(romi_rover_unit_tests_oquam_factory ${SRCS_OQUAMFACTORY})
target_link_libraries(romi_rover_unit_tests_oquam_factory rpp romirover romiserial gtest gmock)

add_test(
    NAME romi_rover_unit_tests_oquam_factory
    COMMAND romi_rover_unit_tests_oquam_factory
)



#set(SRCS_OQUAMSTEPPER
#  src/tests_main.cpp
#  src/tests_oquamstepper.cpp
#  ../src/CNCController.h
#  ../src/StepperController.h
#  ../src/StepperController.cpp
#  ../src/Oquam.h
#  ../src/Oquam.cpp
#  ../src/plotter.cpp
#  ../src/plotter.h
#  ../src/print.h
#  ../src/print.cpp
#  ../src/is_valid.cpp
#  ../src/is_valid.h
#  ../src/Section.h
#  ../src/Section.cpp
#  ../src/ATDC.h
#  ../src/ATDC.cpp
#  ../src/Script.h
#  ../src/Script.cpp
#  ../../arduino_libraries/RomiSerial/IInputStream.h
#  ../../arduino_libraries/RomiSerial/IOutputStream.h
#  ../../arduino_libraries/RomiSerial/RSerial.h
#  ../../arduino_libraries/RomiSerial/RSerial.cpp
#  ../../arduino_libraries/RomiSerial/CRC8.h
#  ../../arduino_libraries/RomiSerial/EnvelopeParser.h
#  ../../arduino_libraries/RomiSerial/EnvelopeParser.cpp
#  ../../arduino_libraries/RomiSerial/RomiSerialClient.h
#  ../../arduino_libraries/RomiSerial/RomiSerialClient.cpp
#  )
#
#add_executable(romi_rover_hardware_tests_oquam ${SRCS_OQUAMSTEPPER})
#target_link_libraries(romi_rover_hardware_tests_oquam rpp romirover romi gtest gmock)



set(SRCS_HARDWARE
  src/tests_main.cpp
  src/tests_hardware.cpp
  )

add_executable(romi_rover_hardware_tests_oquam ${SRCS_HARDWARE})
target_link_libraries(romi_rover_hardware_tests_oquam rpp romirover romiserial gtest gmock)



if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_oquam_coverage
            EXECUTABLE ctest -V ${n_cores}
            EXCLUDE "src/main.cpp" "src/FakeCNCController.h"
            DEPENDENCIES romi_rover_unit_tests_oquam_factory)
endif()

