cmake_minimum_required(VERSION 2.6)
project(libromi)

set(romi_VERSION_MAJOR 0)
set(romi_VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE DEBUG)

set(SOURCES
  src/vector.c 
  src/quaternion.c 
  src/image.c 
  src/fsdb.c 
  src/rover.c)

include_directories(include)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(romi SHARED ${SOURCES})
target_link_libraries(romi r jpeg png m)

INSTALL(TARGETS romi DESTINATION "lib")
INSTALL (
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h")
