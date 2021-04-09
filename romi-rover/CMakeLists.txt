
cmake_minimum_required(VERSION 3.10)
project(romi_rover)

MESSAGE("Romi_rover CXX Flags = ${CMAKE_CXX_FLAGS}")
MESSAGE("Romi_rover C Flags = ${CMAKE_C_FLAGS}")

#option(WITH_REALSENSE "Compile the realsense app" OFF)
#option(WITH_PICAMERA "Compile the picamera app" OFF)

add_subdirectory(weeder2)
add_subdirectory(user-interface)
add_subdirectory(navigation2)
add_subdirectory(oquam)
add_subdirectory(romi-rover)

#add_subdirectory(camera_recorder)
#add_subdirectory(grbl)
#add_subdirectory(webproxy)
#add_subdirectory(gimbal_bldc)

#if(WITH_PICAMERA)
#  add_subdirectory(picamera)
#endif(WITH_PICAMERA)

install(DIRECTORY scripts DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
install(DIRECTORY config DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
