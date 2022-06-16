cmake_minimum_required(VERSION 3.10)
project(romi-rover)

set(SOURCES_TEST
        src/test_harness/main_client_test.cpp
  )
add_executable(romi-remote-client-testharness ${SOURCES_TEST})
target_link_libraries(romi-remote-client-testharness romi rover rcom gmock romimocks)

set(SOURCES_CLIENT
        src/main_client.cpp
        )

add_executable(romi-remote-client ${SOURCES_CLIENT})
target_link_libraries(romi-remote-client romi rover rcom)

