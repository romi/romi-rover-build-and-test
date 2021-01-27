cmake_minimum_required(VERSION 2.6)
project(romi-rover)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
  src/main.cpp
  )

add_executable(romi-rover ${SOURCES})
target_link_libraries(romi-rover romi)

install(TARGETS romi-rover DESTINATION bin)
