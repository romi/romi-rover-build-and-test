cmake_minimum_required(VERSION 2.6)
project(oquam)

set(oquam_VERSION_MAJOR 0)
set(oquam_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

add_executable(oquam src/oquam.c)
target_link_libraries(oquam r m)


