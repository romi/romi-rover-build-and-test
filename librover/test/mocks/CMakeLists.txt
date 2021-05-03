
project(rovermocks)
#
set(SOURCES
        )

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(rovermocks SHARED ${SOURCES})

target_include_directories(rovermocks
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}"
        )