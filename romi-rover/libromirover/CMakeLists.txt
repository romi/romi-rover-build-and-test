cmake_minimum_required(VERSION 2.6)
project(libromirover)

set(libromirover_VERSION_MAJOR 0)
set(libromirover_VERSION_MINOR 1)

set(SOURCES
  include/CNCRange.h
  include/ConfigurationFile.h
  include/DebugWeedingSession.h
  include/ICamera.h
  include/ICNC.h
  include/IConfiguration.h
  include/IFileCabinet.h
  include/IFolder.h
  include/Image.h
  include/image_impl.h
  include/INavigation.h
  include/JSONConfiguration.h
  include/Path.h 
  src/CNCRange.cpp
  src/DebugWeedingSession.cpp
  src/image_impl.c
)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(romirover SHARED ${SOURCES})

target_include_directories(romirover
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

target_link_libraries(romirover
                        rpp
                        jpeg
                        png
                        m)
                      
#if(BUILD_TESTS)
#    add_subdirectory(test)
#endif()
#
#INSTALL(TARGETS romi DESTINATION "lib")
#INSTALL (
#    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
#    DESTINATION include
#    FILES_MATCHING PATTERN "*.h")
