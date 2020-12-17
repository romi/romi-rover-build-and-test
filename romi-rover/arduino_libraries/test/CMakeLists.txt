
cmake_minimum_required(VERSION 3.10)

set(SRCS_ENVELOPE_PARSER
  src/tests_main.cpp
  src/test_envelope_parser.cpp
  ../RomiSerial/RomiSerialErrors.h
  ../RomiSerial/EnvelopeParser.cpp
  ../RomiSerial/EnvelopeParser.h
  ../RomiSerial/CRC8.h)

add_executable(romi_rover_unit_tests_romiserial_envelope_parser ${SRCS_ENVELOPE_PARSER})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(romi_rover_unit_tests_romiserial_envelope_parser r gtest gmock)

add_test(
    NAME romi_rover_unit_tests_romiserial_envelope_parser
    COMMAND romi_rover_unit_tests_romiserial_envelope_parser
)




set(SRCS_MESSAGE_PARSER
  src/tests_main.cpp
  src/test_message_parser.cpp
  ../RomiSerial/RomiSerialErrors.h
  ../RomiSerial/MessageParser.cpp
  ../RomiSerial/MessageParser.h)

add_executable(romi_rover_unit_tests_romiserial_message_parser ${SRCS_MESSAGE_PARSER})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(romi_rover_unit_tests_romiserial_message_parser r gtest gmock)

add_test(
    NAME romi_rover_unit_tests_romiserial_message_parser
    COMMAND romi_rover_unit_tests_romiserial_message_parser
)



set(SRCS_ROMISERIAL
  src/tests_main.cpp
  src/test_romiserial.cpp
  mock/mock_inputstream.h
  mock/mock_outputstream.h
  ../RomiSerial/RomiSerialErrors.h
  ../RomiSerial/IInputStream.h
  ../RomiSerial/IOutputStream.h
  ../RomiSerial/DummySerial.cpp
  ../RomiSerial/IRomiSerial.h
  ../RomiSerial/RomiSerial.cpp
  ../RomiSerial/RomiSerial.h
  ../RomiSerial/MessageParser.cpp
  ../RomiSerial/MessageParser.h
  ../RomiSerial/EnvelopeParser.cpp
  ../RomiSerial/EnvelopeParser.h
  ../RomiSerial/CRC8.h)

add_executable(romi_rover_unit_tests_romiserial ${SRCS_ROMISERIAL})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(romi_rover_unit_tests_romiserial r gtest gmock)

add_test(
    NAME romi_rover_unit_tests_romiserial
    COMMAND romi_rover_unit_tests_romiserial
)



set(SRCS_ROMISERIALCLIENT
  src/tests_main.cpp
  src/test_romiserialclient.cpp
  mock/mock_inputstream.h
  mock/mock_outputstream.h
  ../RomiSerial/RomiSerialErrors.h
  ../RomiSerial/IInputStream.h
  ../RomiSerial/IOutputStream.h
  ../RomiSerial/DummySerial.cpp
  ../RomiSerial/IRomiSerialClient.h
  ../RomiSerial/RomiSerialClient.cpp
  ../RomiSerial/RomiSerialClient.h
  ../RomiSerial/EnvelopeParser.cpp
  ../RomiSerial/EnvelopeParser.h
  ../RomiSerial/CRC8.h)

add_executable(romi_rover_unit_tests_romiserialclient ${SRCS_ROMISERIALCLIENT})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(romi_rover_unit_tests_romiserialclient r gtest gmock)

add_test(
    NAME romi_rover_unit_tests_romiserialclient
    COMMAND romi_rover_unit_tests_romiserialclient
)



set(SRCS_ROMISERIAL_ARDUINO
  src/tests_main.cpp
  src/test_romiserial_arduino.cpp
  ../RomiSerial/IInputStream.h
  ../RomiSerial/IOutputStream.h
  ../RomiSerial/RSerial.cpp
  ../RomiSerial/RSerial.h
  ../RomiSerial/CRC8.h
  ../RomiSerial/EnvelopeParser.cpp
  ../RomiSerial/EnvelopeParser.h
  ../RomiSerial/RomiSerialErrors.h
  ../RomiSerial/RomiSerialClient.cpp
  ../RomiSerial/RomiSerialClient.h)

add_executable(r_test_romiserial_arduino ${SRCS_ROMISERIAL_ARDUINO})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(r_test_romiserial_arduino r gtest gmock)
