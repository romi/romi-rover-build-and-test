project(libroverfakes)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../include/weeder )
#
set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/FakeSVMSegmentation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FakeImageCropper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FakeConnectedComponents.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FakePipelineFactory.cpp
)

add_library(roverfakes SHARED ${SOURCES})

target_link_libraries(roverfakes
        romi)

target_include_directories(roverfakes
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
        )