cmake_minimum_required(VERSION 3.10)
project(romi-cablebot)

add_executable(romi-cablebot main.cpp)
target_link_libraries(romi-cablebot romi rcom)

if(${PI_BUILD})
  add_executable(romi-cablebot-cmdline main-cmdline.cpp)
  target_link_libraries(romi-cablebot-cmdline romi rcom)
endif()

install(TARGETS romi-cablebot DESTINATION bin)
