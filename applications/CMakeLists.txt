
cmake_minimum_required(VERSION 3.10)
project(romi_rover-applications)

############################################################

output_compile_flags()

############################################################

add_subdirectory(weeder)
add_subdirectory(romi-rover)
add_subdirectory(rcdiscover)
add_subdirectory(romi-remote-client)
add_subdirectory(batterymonitor)
add_subdirectory(romi-cnc)
add_subdirectory(romi-camera)
add_subdirectory(romi-cablebot)
add_subdirectory(romi-config)



#install(DIRECTORY scripts DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
#install(DIRECTORY config DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
