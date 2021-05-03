cmake_minimum_required(VERSION 3.10)

include_directories(include
        src)

add_executable(rcdiscover
                src/main.cpp
                include/DeviceLister.h
                src/DeviceLister.cpp
                include/PortConfigurationGenerator.h
                src/PortConfigurationGenerator.cpp
                include/SerialPortIdentification.h
                src/SerialPortIdentification.cpp
                include/SerialPortDiscover.h
                src/SerialPortDiscover.cpp
        )

target_link_libraries( rcdiscover
                        rpp
                        romiserial
                        stdc++fs)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()
