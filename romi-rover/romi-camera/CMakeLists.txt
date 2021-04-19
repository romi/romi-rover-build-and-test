cmake_minimum_required(VERSION 3.10)
project(romi-camera)

add_executable(romi-camera main.cpp)
target_link_libraries(romi-camera romi rcom)
install(TARGETS romi-camera DESTINATION bin)
