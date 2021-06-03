cmake_minimum_required(VERSION 3.10)
project(weeder)

include_directories( "src" )

add_executable(weeder src/main.cpp)

target_link_libraries(weeder rcompp-lite rpp  romi rover)

add_executable(weeder_eval src/eval.cpp)

target_link_libraries(weeder_eval rcompp-lite rpp m  romi rover roverfakes)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

#ADD_CUSTOM_COMMAND(
#  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp
#  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  )

install(TARGETS weeder DESTINATION bin/romi)
