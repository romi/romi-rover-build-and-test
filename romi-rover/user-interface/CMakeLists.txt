cmake_minimum_required(VERSION 2.6)
project(user-interface)

set(SOURCES
  src/main.cpp
  src/UIFactory.h
  src/UIFactory.cpp
  )

add_executable(user-interface ${SOURCES})
target_link_libraries(user-interface romi)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

install(TARGETS user-interface DESTINATION bin/romi)
