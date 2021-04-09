cmake_minimum_required(VERSION 3.10)
project(weeder2)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src" )

add_executable(weeder2 src/main.cpp)

target_link_libraries(weeder2 rcompp rpp romi)

add_executable(weeder2_eval src/eval.cpp)

target_link_libraries(weeder2_eval rcompp rpp romi m)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

#ADD_CUSTOM_COMMAND(
#  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp
#  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  )

install(TARGETS weeder2 DESTINATION bin/romi)
