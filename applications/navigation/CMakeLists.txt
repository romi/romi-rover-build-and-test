cmake_minimum_required(VERSION 3.10)
project(navigation)

add_executable(navigation src/main.cpp)

target_link_libraries(navigation romi romiserial rcompp-lite rpp)

install(TARGETS navigation DESTINATION bin/romi)
