cmake_minimum_required(VERSION 3.10)
project(navigation2)

add_executable(navigation2 src/main.cpp)

target_link_libraries(navigation2 romi romiserial rcompp rpp)

install(TARGETS navigation2 DESTINATION bin/romi)
