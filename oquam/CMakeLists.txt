cmake_minimum_required(VERSION 2.6)
project(oquam)

set(oquam_VERSION_MAJOR 0)
set(oquam_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

set(SOURCES
  src/action.c
  src/script.c
  src/controller.c
  src/stepper.c
  src/virt.c
  src/ostepper.c
  src/cnc.c
  src/planner.c
  src/plotter.c
  src/v.c)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/../src")

add_library(liboquam SHARED ${SOURCES})
target_link_libraries(liboquam r m)

add_executable(oquam_node oquam.c oquam_main.c)
target_link_libraries(oquam_node liboquam rcom)

add_executable(oquam src/oquam.c)
target_link_libraries(oquam liboquam)

add_executable(test1 tests/test1.c)
target_link_libraries(test1 liboquam)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/oquam_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/oquam_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )


