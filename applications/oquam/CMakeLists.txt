cmake_minimum_required(VERSION 3.10)
project(oquam)

set(SOURCES
  main.cpp
  OquamFactory.cpp
  OquamFactory.h
  )

add_executable(oquam ${SOURCES})
target_link_libraries(oquam romi romiserial)

#add_subdirectory(test)


