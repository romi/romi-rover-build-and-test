cmake_minimum_required(VERSION 3.10)

project(librover)
set(librover_VERSION_MAJOR 0)
set(librover_VERSION_MINOR 1)

set(SOURCES
        include/constraintsolver/GConstraintSolver.h
        include/quincunx/point.h
        include/quincunx/Quincunx.h
        include/svm/SVMSegmentation.h
        include/som/centres.h
        include/som/fixed.h
        include/som/Real.h
        include/som/SelfOrganizedMap.h
        include/som/slic.h
        include/som/SOM.h
        include/som/Superpixels.h
        include/weeder/Weeder.h
        include/weeder/IImageSegmentation.h
        include/weeder/IPathPlanner.h
        include/weeder/IPipeline.h
        include/weeder/PipelineFactory.h
        include/weeder/Pipeline.h

        src/constraintsolver/GConstraintSolver.cpp
        src/quincunx/Quincunx.cpp

        src/svm/SVMSegmentation.cpp
        src/som/SOM.cpp
        src/som/Superpixels.cpp
        src/som/fixed.c
        src/som/slic.cpp

        src/weeder/Weeder.cpp
        src/weeder/Pipeline.cpp
        src/weeder/PipelineFactory.cpp)


include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(rover SHARED ${SOURCES})

target_include_directories(rover
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

target_link_libraries(rover pthread romi)

# Always build the mocks library.
#add_subdirectory(test/mocks)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
