
cmake_minimum_required(VERSION 3.10)

set(SRCS_PARSER
  src/tests_main.cpp
  src/test_parser.cpp
  ../RomiSerial/RomiSerialErrors.h
  ../RomiSerial/IParser.h
  ../RomiSerial/Parser.cpp
  ../RomiSerial/Parser.h
  ../RomiSerial/CRC8.h)

add_executable(r_unit_tests_romiserial_parser ${SRCS_PARSER})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(r_unit_tests_romiserial_parser gtest gmock)

add_test(
    NAME r_unit_tests_romiserial_parser
    COMMAND r_unit_tests_romiserial_parser
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
  ../RomiSerial/Parser.cpp
  ../RomiSerial/Parser.h
  ../RomiSerial/CRC8.h)

add_executable(r_unit_tests_romiserial ${SRCS_ROMISERIAL})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(r_unit_tests_romiserial gtest gmock)

add_test(
    NAME r_unit_tests_romiserial
    COMMAND r_unit_tests_romiserial
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
  ../RomiSerial/CRC8.h)

add_executable(r_unit_tests_romiserialclient ${SRCS_ROMISERIALCLIENT})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(r_unit_tests_romiserialclient r gtest gmock)

add_test(
    NAME r_unit_tests_romiserialclient
    COMMAND r_unit_tests_romiserialclient
)




set(SRCS_PARSER
  ../RomiSerial/IInputStream.h
  ../RomiSerial/IOutputStream.h
  ../RomiSerial/RSerial.cpp
  ../RomiSerial/RSerial.h
  ../RomiSerial/CRC8.h
  ../RomiSerial/RomiSerialErrors.h
  ../RomiSerial/RomiSerialClient.cpp
  ../RomiSerial/RomiSerialClient.h
  src/test_romiserial_arduino.cpp)

add_executable(r_test_romiserial_arduino ${SRCS_PARSER})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../RomiSerial" )

target_link_libraries(r_test_romiserial_arduino r gtest gmock)
