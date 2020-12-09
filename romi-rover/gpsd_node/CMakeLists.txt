cmake_minimum_required(VERSION 2.6)
project(gpsd_node)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(gpsd_node gpsd_node.cpp gpsd_main.cpp)
target_link_libraries(gpsd_node -lgps romi librcom r m)
#install(TARGETS gpsd DESTINATION bin/romi)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/gpsd_main.cpp
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/gpsd_main.cpp ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
