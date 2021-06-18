cmake_minimum_required(VERSION 3.10)
project(weeder)

add_executable(weeder_eval eval.cpp)
target_link_libraries(weeder_eval rcompp-lite rpp m  romi rover roverfakes)
