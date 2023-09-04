cmake_minimum_required(VERSION 3.10)
project(romi-config)

add_executable(romi-config main.cpp)
target_link_libraries(romi-config romi rcom)
install(TARGETS romi-config DESTINATION bin)
