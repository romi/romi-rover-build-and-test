cmake_minimum_required(VERSION 2.6)
project(navigation2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../arduino_libraries/RomiSerial" )

add_executable(navigation2
  src/main.cpp
  src/IMotorDriver.h
  src/BrushMotorDriver.h
  src/BrushMotorDriver.cpp
  src/IConfiguration.h
  src/ConfigurationFile.h
  src/WheelOdometry.h
  src/WheelOdometry.cpp
  src/INavigation.h
  src/Navigation.h
  src/Navigation.cpp
  src/RPCNavigationServerAdaptor.h
  src/RPCNavigationServerAdaptor.cpp
  )

target_link_libraries(navigation2 rcompp rpp romiserial m)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

install(TARGETS navigation2 DESTINATION bin/romi)
