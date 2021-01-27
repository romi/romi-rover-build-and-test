cmake_minimum_required(VERSION 2.6)
project(navigation2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(navigation2 src/main.cpp)

target_link_libraries(navigation2 romi romiserial rcompp rpp)

install(TARGETS navigation2 DESTINATION bin/romi)
