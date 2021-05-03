cmake_minimum_required(VERSION 3.10)

include_directories(mocks
        support)

set(SRCS
        support/TestUtil.hpp
        src/tests_main.cpp
        src/DeviceLister_tests.cpp
        src/SerialPortIdentification_tests.cpp
        src/SerialPortDiscover_tests.cpp
        src/PortConfigurationGenerator_tests.cpp
        ../include/IPortConfigurationGenerator.h
        ../include/PortConfigurationGenerator.h
        ../src/PortConfigurationGenerator.cpp
        ../include/ISerialPortIdentification.h
        ../include/SerialPortIdentification.h
        ../src/SerialPortIdentification.cpp
        ../include/ISerialPortDiscover.h
        ../include/SerialPortDiscover.h
        ../src/SerialPortDiscover.cpp
        ../include/DeviceLister.h
        ../src/DeviceLister.cpp
        )

add_executable(romi_rover_rcdiscover_unit_tests ${SRCS})

target_link_libraries(  romi_rover_rcdiscover_unit_tests
                        gtest
                        gmock
                        romiserial
                        rpp
                        r
                        stdc++fs)

add_test(
        NAME romi_rover_rcdiscover_unit_tests
        COMMAND romi_rover_rcdiscover_unit_tests
        )
