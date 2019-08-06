cmake_minimum_required(VERSION 2.6)
project(libromi)

set(romi_VERSION_MAJOR 0)
set(romi_VERSION_MINOR 1)

set(SOURCES
  src/list.c 
  src/vector.c 
  src/quaternion.c 
  src/image.c 
  src/fs.c 
  src/fsdb.c 
  src/rover.c)

include_directories(include)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(romi SHARED ${SOURCES})
target_link_libraries(romi jpeg m)

INSTALL(TARGETS romi DESTINATION "lib")
INSTALL (
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h")
