cmake_minimum_required(VERSION 3.10)
project(gimbal_bldc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(gimbal_bldc gimbal_bldc.c gimbal_bldc_main.c)
target_link_libraries(gimbal_bldc romi rcom r m)
#install(TARGETS gimbal_bldc DESTINATION bin/romi)

#if(BUILD_TESTS)
#    add_subdirectory(test)
#endif()

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/gimbal_bldc_main.c
  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/gimbal_bldc_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
  )
