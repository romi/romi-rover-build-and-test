cmake_minimum_required(VERSION 3.10)
project(romi-cnc)

set(SOURCES
  main.cpp
  OquamFactory.cpp
  OquamFactory.h
  )

add_executable(romi-cnc ${SOURCES})
target_link_libraries(romi-cnc romi romiserial)

#add_subdirectory(test)


