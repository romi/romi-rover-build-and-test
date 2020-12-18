
cmake_minimum_required(VERSION 3.10)

#set(SRCS_JSON src/tests_main.cpp src/test_json.cpp)
#add_executable(r_unit_tests_weeder2_json ${SRCS_JSON})
#include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/.." )
#target_link_libraries(r_unit_tests_weeder2_json gtest gmock r romi)


set(SRCS_JSON som/elastic.cpp som/elastic.cpp som/test.c ../src/som/fixed.c)
add_executable(romi_rover_regression_test_som ${SRCS_JSON})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_regression_test_som r romi romirover)


add_executable(romi_rover_regression_test_som_compare som/compare.c)


#add_test(
#    NAME r_unit_tests_weeder2_json
#    COMMAND r_unit_tests_weeder2_json
#)

#if(BUILD_COVERAGE)
#    SETUP_TARGET_FOR_COVERAGE_LCOV(
#            NAME romi_rover_unit_tests_weeder2_json
#            EXECUTABLE ctest -V ${n_cores}
#            DEPENDENCIES romi_rover_unit_tests_weeder2_json)
#endif()

