cmake_minimum_required(VERSION 2.6)
project(oquam)

set(oquam_VERSION_MAJOR 0)
set(oquam_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
  src/main.cpp
  src/OquamFactory.cpp
  src/OquamFactory.h
  )

add_executable(oquam ${SOURCES})
target_link_libraries(oquam rcompp romi romiserial)
target_include_directories(oquam PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
install(TARGETS oquam DESTINATION bin/romi)

#add_executable(oquam_test src/oquam_test.cpp)
#target_link_libraries(oquam_test liboquam)
#install(TARGETS oquam_test DESTINATION bin/romi)

#install(DIRECTORY src DESTINATION include/oquam FILES_MATCHING PATTERN "*.h")
#install(DIRECTORY src DESTINATION include/oquam FILES_MATCHING PATTERN "*.hpp")

#ADD_CUSTOM_COMMAND(
#  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/oquam_main.c
#  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/oquam_main.c ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  )


add_subdirectory(test)


