cmake_minimum_required(VERSION 3.10)
project(romi-cablebot)

add_executable(romi-cablebot main.cpp)
add_executable(romi-cablebot-cmdline main-cmdline.cpp)
target_link_libraries(romi-cablebot romi rcom)
target_link_libraries(romi-cablebot-cmdline romi rcom)
install(TARGETS romi-cablebot DESTINATION bin)
