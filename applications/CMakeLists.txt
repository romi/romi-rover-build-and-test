
cmake_minimum_required(VERSION 3.10)
project(romi_rover-applications)

############################################################

output_compile_flags()

############################################################

add_subdirectory(weeder)
add_subdirectory(romi-rover)
add_subdirectory(rcdiscover)
add_subdirectory(scratch-test)

if(${PI_BUILD})
    add_subdirectory(romi-camera)
endif()


#install(DIRECTORY scripts DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
#install(DIRECTORY config DESTINATION share/romi FILES_MATCHING PATTERN "*.json")
