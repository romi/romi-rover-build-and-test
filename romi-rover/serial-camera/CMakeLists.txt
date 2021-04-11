cmake_minimum_required(VERSION 2.6)
project(serial-camera)

add_executable(serial-camera main.cpp)
target_link_libraries(serial-camera romi romiserial)
install(TARGETS serial-camera DESTINATION bin)
