cmake_minimum_required(VERSION 2.6)
project(weeder2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src" )

add_executable(weeder2 src/main.cpp)

target_link_libraries(weeder2 rcompp rpp romirover)


add_executable(weeder2_eval src/eval.cpp)

target_link_libraries(weeder2_eval rcompp rpp romirover m)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

#ADD_CUSTOM_COMMAND(
#  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp
#  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  )

install(TARGETS weeder2 DESTINATION bin/romi)
