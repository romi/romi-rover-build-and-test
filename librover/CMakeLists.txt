cmake_minimum_required(VERSION 3.10)
project(librover)

############################################################

output_compile_flags()

############################################################

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
        include/som/SOM.h
        include/som/Superpixels.h
        include/unet/PythonSegmentation.h
        include/unet/PythonSVM.h
        include/unet/PythonUnet.h
        include/unet/UnetImager.h
        include/weeder/Weeder.h
        include/weeder/IConnectedComponents.h
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

        src/unet/PythonSegmentation.cpp
        src/unet/PythonSVM.cpp
        src/unet/PythonUnet.cpp
        src/unet/UnetImager.cpp

        src/astar/AStar.cpp
        
        src/weeder/ConnectedComponents.cpp
        src/weeder/Pipeline.cpp
        src/weeder/PipelineFactory.cpp
        src/weeder/Weeder.cpp
        )


include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library(rover SHARED ${SOURCES})

target_include_directories(rover
        PUBLIC
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )
      
# if((CMAKE_SYSTEM_PROCESSOR MATCHES ".*arm") OR (CMAKE_SYSTEM_PROCESSOR MATCHES ".*aarch64"))
#     set(PICAMERA_LIBRARIES mmal_core mmal_util mmal vcos)
# endif()

target_link_libraries(rover pthread romi)

# Always build the mocks library.
add_subdirectory(test/fakes)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
