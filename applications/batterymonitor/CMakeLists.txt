cmake_minimum_required(VERSION 3.14)
project(romi-rover)

set(SOURCES
        src/main.cpp
        src/VeDirectFrameHandler.cpp
  )

add_executable(batterymonitor ${SOURCES})
target_link_libraries(batterymonitor romi rover rcom romimocks)


