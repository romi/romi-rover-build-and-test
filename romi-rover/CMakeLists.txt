
cmake_minimum_required(VERSION 2.6)
project(romi_rover)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default option is off.
# We override this on the server for a test buid, so tests should be run with this on before checking in.
option(ADDRESS_SANITISER_BUILD "Enable build settings to allow address sanitiser" ON) # default to off

if(ADDRESS_SANITISER_BUILD)

  message(WARNING "Enabling GCC address sanitiser will disable all Valgrind tasks due to compatibility errors")
  set( PROJECT_SANITISE_FLAGS
          "-fsanitize=address,undefined,shift,integer-divide-by-zero,unreachable,vla-bound,null,return,signed-integer-overflow,bounds,alignment,object-size,float-divide-by-zero,float-cast-overflow,nonnull-attribute,returns-nonnull-attribute,bool,enum,vptr")
else()
  MESSAGE("libr sanitize flags off")
  set( PROJECT_SANITISE_FLAGS "")
endif()

# For now everything is debug. Set Compile options locally to maintain independent library builds.
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter ${PROJECT_SANITISE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter ${PROJECT_SANITISE_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "-O0")

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROJECT_SANITISE_FLAGS}")

############################################################

option(WITH_REALSENSE "Compile the realsense app" OFF)
option(WITH_PICAMERA "Compile the picamera app" OFF)

add_subdirectory(brush_motors)
add_subdirectory(camera_recorder)
add_subdirectory(configuration)
add_subdirectory(fake_camera)
add_subdirectory(fake_cnc)
add_subdirectory(fake_motors)
add_subdirectory(grbl)
#add_subdirectory(interface)
add_subdirectory(script_engine)
add_subdirectory(navigation)
add_subdirectory(video4linux)
add_subdirectory(weeder)
add_subdirectory(wheel_odometry)
add_subdirectory(fsdb)
add_subdirectory(watchdog)
add_subdirectory(webproxy)
add_subdirectory(rclogger)
add_subdirectory(control_panel)
add_subdirectory(gimbal_servo)
add_subdirectory(oquam)

if(WITH_REALSENSE)
  add_subdirectory(realsense)
endif(WITH_REALSENSE)

if(WITH_PICAMERA)
  add_subdirectory(picamera)
endif(WITH_PICAMERA)

install(DIRECTORY scripts DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
install(DIRECTORY config DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
