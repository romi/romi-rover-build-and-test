cmake_minimum_required(VERSION 2.6)
project(oquam)

set(oquam_VERSION_MAJOR 0)
set(oquam_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

set(SOURCES
  src/script.c
  src/script.h
  src/Controller.hpp
  src/StepperController.cpp
  src/StepperController.hpp
  src/VirtualStepperController.cpp
  src/VirtualStepperController.hpp
  src/OquamStepperController.cpp
  src/OquamStepperController.hpp
  src/planner.c
  src/planner.h
  src/plotter.c
  src/plotter.h
  src/v.c
  src/v.h)

add_library(liboquam SHARED ${SOURCES})
target_link_libraries(liboquam r m)

add_executable(oquam oquam.cpp oquam.hpp oquam_main.c)
target_link_libraries(oquam liboquam rcom)
target_include_directories(oquam PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
install(TARGETS oquam DESTINATION romi)

add_executable(oquam_test src/oquam_test.cpp)
target_link_libraries(oquam_test liboquam)
install(TARGETS oquam_test DESTINATION romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/oquam_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/oquam_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )


