
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )


set(SRCS_SCRIPT
  src/tests_main.cpp
  src/tests_script.cpp
  ../src/IScript.h
  ../src/Script.h
  ../src/Action.h)

add_executable(r_unit_tests_oquam_script ${SRCS_SCRIPT})
target_link_libraries(r_unit_tests_oquam_script gtest gmock)

add_test(
    NAME r_unit_tests_oquam_script
    COMMAND r_unit_tests_oquam_script
)




set(SRCS_PLANNER
  src/tests_main.cpp
  src/tests_planner.cpp
  ../src/IScript.h
  ../src/Script.h
  ../src/Action.h
  ../src/ATDC.h
  ../src/Section.h
  ../src/v.h
  ../src/v.c
  ../src/Planner.h
  ../src/Planner.cpp)

add_executable(r_unit_tests_oquam_planner ${SRCS_PLANNER})
target_link_libraries(r_unit_tests_oquam_planner gtest gmock)

add_test(
    NAME r_unit_tests_oquam_planner
    COMMAND r_unit_tests_oquam_planner
)

