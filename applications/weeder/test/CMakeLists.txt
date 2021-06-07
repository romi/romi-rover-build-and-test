
cmake_minimum_required(VERSION 3.10)

set(SRCS_ELASTIC som/elastic.cpp som/elastic.cpp som/test.c)
add_executable(romi_rover_regression_test_som ${SRCS_ELASTIC})
include_directories( "../src" )
target_link_libraries(romi_rover_regression_test_som r romi rover)

add_executable(romi_rover_regression_test_som_compare som/compare.c)


