cmake_minimum_required(VERSION 3.10)
project(oquam)

set(oquam_VERSION_MAJOR 0)
set(oquam_VERSION_MINOR 1)

set(SOURCES
  src/main.cpp
  src/OquamFactory.cpp
  src/OquamFactory.h
  )

add_executable(oquam ${SOURCES})
target_link_libraries(oquam rcompp-lite romi romiserial)
target_include_directories(oquam PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
install(TARGETS oquam DESTINATION bin/romi)

add_subdirectory(test)


