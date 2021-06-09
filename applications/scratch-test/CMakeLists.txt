cmake_minimum_required(VERSION 3.10)
project(romi-rover)

set(SOURCES
  src/main.cpp
  )

add_executable(scratch-test ${SOURCES})
target_link_libraries(scratch-test romi rover)

