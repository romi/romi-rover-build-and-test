cmake_minimum_required(VERSION 2.6)
project(joystick)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(joystick joystick.cpp joystick_main.cpp)
target_link_libraries(joystick romi rcom r m)
#install(TARGETS joystick DESTINATION bin/romi)

#if(BUILD_TESTS)
#    add_subdirectory(test)
#endif()

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/joystick_main.cpp
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/joystick_main.cpp ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
