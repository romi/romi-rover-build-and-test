cmake_minimum_required(VERSION 2.6)
project(weeder2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(weeder2 weeder.cpp weeder_main.cpp)
target_link_libraries(weeder2 rcom romi r m)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )

install(TARGETS weeder2 DESTINATION bin/romi)
