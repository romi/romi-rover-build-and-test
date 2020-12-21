
cmake_minimum_required(VERSION 2.6)
project(romiserial)

set(romiserial_VERSION_MAJOR 0)
set(romiserial_VERSION_MINOR 1)

set(SOURCES
  RomiSerial/IInputStream.h
  RomiSerial/IOutputStream.h
  RomiSerial/CRC8.h
  RomiSerial/EnvelopeParser.h
  RomiSerial/EnvelopeParser.cpp
  RomiSerial/IRomiSerialClient.h
  RomiSerial/RomiSerialClient.h
  RomiSerial/RomiSerialClient.cpp
  RomiSerial/RSerial.h
  RomiSerial/RSerial.cpp
)

add_library(romiserial SHARED ${SOURCES})

target_include_directories(romiserial
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/RomiSerial"
        )

target_link_libraries(romiserial rpp)

add_subdirectory(test)
