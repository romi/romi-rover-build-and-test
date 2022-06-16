cmake_minimum_required(VERSION 3.10)
project(romi-cablebot)

add_executable(romi-cablebot main.cpp)
target_link_libraries(romi-cablebot romi rcom)
install(TARGETS romi-cablebot DESTINATION bin)
