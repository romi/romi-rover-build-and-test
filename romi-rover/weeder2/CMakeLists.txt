cmake_minimum_required(VERSION 2.6)
project(weeder2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src" )

add_executable(weeder2
  src/IImageCropper.h
  src/ImageCropperFactory.h
  src/ImageCropperFactory.cpp
  src/ImageCropper.h
  src/ImageCropper.cpp  
  src/IImageSegmentation.h
  src/ImageSegmentationFactory.h
  src/ImageSegmentationFactory.cpp
  src/RPCCNCClientAdaptor.h
  src/RPCCNCClientAdaptor.cpp
  src/svm/SVMSegmentation.h
  src/svm/SVMSegmentation.cpp
  src/quincunx/Quincunx.h
  src/quincunx/Quincunx.cpp
  src/quincunx/point.h
  src/quincunx/point.c
  src/som/SOM.cpp 
  src/som/Superpixels.cpp 
  src/som/slic.cpp 
  src/som/fixed.c 
  src/IPathPlanner.h
  src/PathPlannerFactory.h
  src/PathPlannerFactory.cpp
  src/IPipeline.h
  src/Pipeline.h
  src/Pipeline.cpp
  src/CameraFactory.cpp
  src/Weeder.h
  src/Weeder.cpp
  src/CameraUSB.cpp
  src/camera_v4l.c
  src/main.cpp)

target_link_libraries(weeder2 rcompp rpp romirover)


add_executable(weeder2_eval
  src/IImageCropper.h
  src/ImageCropperFactory.h
  src/ImageCropperFactory.cpp
  src/ImageCropper.h
  src/ImageCropper.cpp  
  src/IImageSegmentation.h
  src/ImageSegmentationFactory.h
  src/ImageSegmentationFactory.cpp
  src/RPCCNCClientAdaptor.h
  src/RPCCNCClientAdaptor.cpp
  src/svm/SVMSegmentation.h
  src/svm/SVMSegmentation.cpp
  src/quincunx/Quincunx.h
  src/quincunx/Quincunx.cpp
  src/quincunx/point.h
  src/quincunx/point.c
  src/som/SOM.cpp 
  src/som/Superpixels.cpp 
  src/som/slic.cpp 
  src/som/fixed.c 
  src/IPathPlanner.h
  src/PathPlannerFactory.h
  src/PathPlannerFactory.cpp
  src/IPipeline.h
  src/Pipeline.h
  src/Pipeline.cpp
  src/CameraFactory.cpp
  src/Weeder.h
  src/Weeder.cpp
  src/CameraUSB.cpp
  src/camera_v4l.c
  src/eval.cpp)

target_link_libraries(weeder2_eval rcompp rpp romirover m)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

#ADD_CUSTOM_COMMAND(
#  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp
#  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  )

install(TARGETS weeder2 DESTINATION bin/romi)
