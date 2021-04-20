
cmake_minimum_required(VERSION 3.10)
project(romi_rover)

MESSAGE("Romi_rover CXX Flags = ${CMAKE_CXX_FLAGS}")
MESSAGE("Romi_rover C Flags = ${CMAKE_C_FLAGS}")


if((CMAKE_SYSTEM_PROCESSOR MATCHES ".*arm") OR (CMAKE_SYSTEM_PROCESSOR MATCHES ".*aarch64"))
    set(PICAMERA_LIBRARY_PATH /opt/vc/lib)
    link_directories(${PICAMERA_LIBRARY_PATH})
endif()

add_subdirectory(weeder2)
add_subdirectory(user-interface)
add_subdirectory(navigation2)
add_subdirectory(oquam)
add_subdirectory(romi-rover)

if((CMAKE_SYSTEM_PROCESSOR MATCHES ".*arm") OR (CMAKE_SYSTEM_PROCESSOR MATCHES ".*aarch64"))
add_subdirectory(romi-camera)
endif()

#add_subdirectory(camera_recorder)
#add_subdirectory(grbl)
#add_subdirectory(webproxy)
#add_subdirectory(gimbal_bldc)

#if(WITH_PICAMERA)
#  add_subdirectory(picamera)
#endif(WITH_PICAMERA)

install(DIRECTORY scripts DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
install(DIRECTORY config DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
