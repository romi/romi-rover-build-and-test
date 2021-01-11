cmake_minimum_required(VERSION 2.6)
project(user-interface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
  src/main.cpp
  src/UIFactory.h
  src/UIFactory.cpp
  )

add_executable(user-interface ${SOURCES})
target_link_libraries(user-interface romirover)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

install(TARGETS user-interface DESTINATION bin/romi)
