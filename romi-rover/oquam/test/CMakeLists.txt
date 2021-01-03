
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino_libraries/RomiSerial" )


set(SRCS_STEPPERSETTINGS
  src/tests_main.cpp
  src/tests_steppersettings.cpp
  ../src/StepperSettings.h
  ../src/StepperSettings.cpp
  )
add_executable(romi_rover_unit_tests_oquam_steppersettings ${SRCS_STEPPERSETTINGS})
target_link_libraries(romi_rover_unit_tests_oquam_steppersettings rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_oquam_steppersettings
    COMMAND romi_rover_unit_tests_oquam_steppersettings
)


set(SRCS_SECTION
  src/tests_main.cpp
  src/tests_section.cpp
  ../src/Section.h
  ../src/Section.cpp
  ../src/Script.h
  ../src/Script.cpp
  ../src/ATDC.h
  ../src/ATDC.cpp
  ../src/is_valid.cpp
  ../src/is_valid.h
  ../src/print.h
  ../src/print.cpp
  )
add_executable(romi_rover_unit_tests_oquam_section ${SRCS_SECTION})
target_link_libraries(romi_rover_unit_tests_oquam_section rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_oquam_section
    COMMAND romi_rover_unit_tests_oquam_section
)




set(SRCS_PRINT
  src/tests_main.cpp
  src/tests_print.cpp
  ../src/Script.h
  ../src/Script.cpp
  ../src/ATDC.h
  ../src/ATDC.cpp
  ../src/Section.h
  ../src/Section.cpp
  ../src/print.h
  ../src/print.cpp
  ../src/is_valid.cpp
  ../src/is_valid.h
  )
add_executable(romi_rover_unit_tests_oquam_print ${SRCS_PRINT})
target_link_libraries(romi_rover_unit_tests_oquam_print rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_oquam_print
    COMMAND romi_rover_unit_tests_oquam_print
)


set(SRCS_SCRIPT
  src/tests_main.cpp
  src/tests_script.cpp
  ../src/Script.h
  ../src/Script.cpp
  ../src/ATDC.h
  ../src/ATDC.cpp
  ../src/Section.h
  ../src/Section.cpp
  ../src/print.h
  ../src/print.cpp
  ../src/is_valid.cpp
  ../src/is_valid.h
  )
add_executable(romi_rover_unit_tests_oquam_script ${SRCS_SCRIPT})
target_link_libraries(romi_rover_unit_tests_oquam_script rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_oquam_script
    COMMAND romi_rover_unit_tests_oquam_script
)



set(SRCS_ADAPTOR
  src/tests_main.cpp
  src/tests_cncadaptor.cpp
  ../src/CNCAdaptor.h
  ../src/CNCAdaptor.cpp
  )
add_executable(romi_rover_unit_tests_oquam_cncadaptor ${SRCS_ADAPTOR})
target_link_libraries(romi_rover_unit_tests_oquam_cncadaptor rpp romirover rcompp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_oquam_cncadaptor
    COMMAND romi_rover_unit_tests_oquam_cncadaptor
)



set(SRCS_OQUAM
  src/tests_main.cpp
  src/tests_oquam.cpp
  mock/mock_controller.h
  ../src/CNCController.h
  ../src/Oquam.h
  ../src/Oquam.cpp
  ../src/plotter.cpp
  ../src/plotter.h
  ../src/print.h
  ../src/print.cpp
  ../src/is_valid.cpp
  ../src/is_valid.h
  ../src/Section.h
  ../src/Section.cpp
  ../src/ATDC.h
  ../src/ATDC.cpp
  ../src/Script.h
  ../src/Script.cpp
  ../src/Oquam.cpp
  )

add_executable(romi_rover_unit_tests_oquam_oquam ${SRCS_OQUAM})
target_link_libraries(romi_rover_unit_tests_oquam_oquam rpp romirover romi gtest gmock)

add_test(
    NAME romi_rover_unit_tests_oquam_oquam
    COMMAND romi_rover_unit_tests_oquam_oquam
)



set(SRCS_OQUAMOPTIONS
  src/tests_main.cpp
  src/tests_oquamoptions.cpp
  ../src/OquamOptions.h
  ../src/OquamOptions.cpp
  )

add_executable(romi_rover_unit_tests_oquam_options ${SRCS_OQUAMOPTIONS})
target_link_libraries(romi_rover_unit_tests_oquam_options rpp romirover romi gtest gmock)

add_test(
    NAME romi_rover_unit_tests_oquam_options
    COMMAND romi_rover_unit_tests_oquam_options
)


set(SRCS_OQUAMFACTORY
  src/tests_main.cpp
  src/tests_oquamfactory.cpp
  ../src/OquamFactory.h
  ../src/OquamFactory.cpp
  ../src/CNCController.h
  ../src/FakeCNCController.h
  ../src/StepperController.h
  ../src/StepperController.cpp
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

add_executable(romi_rover_unit_tests_oquam_factory ${SRCS_OQUAMFACTORY})
target_link_libraries(romi_rover_unit_tests_oquam_factory rpp romirover romi gtest gmock)

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
  ../src/CNCController.h
  ../src/StepperController.h
  ../src/StepperController.cpp
  ../src/Oquam.h
  ../src/Oquam.cpp
  ../src/plotter.cpp
  ../src/plotter.h
  ../src/print.h
  ../src/print.cpp
  ../src/is_valid.cpp
  ../src/is_valid.h
  ../src/Section.h
  ../src/Section.cpp
  ../src/ATDC.h
  ../src/ATDC.cpp
  ../src/Script.h
  ../src/Script.cpp
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

add_executable(romi_rover_hardware_tests_oquam ${SRCS_HARDWARE})
target_link_libraries(romi_rover_hardware_tests_oquam rpp romirover romi gtest gmock)



if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_oquam_coverage
            EXECUTABLE ctest -V ${n_cores}
            EXCLUDE "src/main.cpp" "src/FakeCNCController.h"
            DEPENDENCIES romi_rover_unit_tests_oquam_section
                         romi_rover_unit_tests_oquam_steppersettings
                         romi_rover_unit_tests_oquam_script
                         romi_rover_unit_tests_oquam_cncadaptor
                         romi_rover_unit_tests_oquam_print
                         romi_rover_unit_tests_oquam_oquam
                         romi_rover_unit_tests_oquam_options
                         romi_rover_unit_tests_oquam_factory)
endif()

