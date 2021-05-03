
cmake_minimum_required(VERSION 3.10)

include_directories( "../src" )

set(SRCS_UIFACTORY 
  src/tests_main.cpp
  src/tests_uifactory.cpp
  ../src/UIFactory.h
  ../src/UIFactory.cpp
 )

add_executable(romi_rover_unit_tests_userinterface_uifactory ${SRCS_UIFACTORY})
target_link_libraries(romi_rover_unit_tests_userinterface_uifactory romi romiserial rcompp-lite rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_userinterface_uifactory
    COMMAND romi_rover_unit_tests_userinterface_uifactory
)


if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_userinterface_coverage
            EXECUTABLE ctest -V ${n_cores}
            EXCLUDE "src/main.cpp" 
            DEPENDENCIES romi_rover_unit_tests_userinterface_uifactory)
endif()

