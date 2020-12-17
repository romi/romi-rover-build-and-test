cmake_minimum_required(VERSION 2.6)
project(weeder2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_executable(weeder2
  Image.h
  Path.h
  CNCRange.h
  IController.h
  IController.h
  ControllerClient.cpp
  ControllerClient.h
  ControllerServer.cpp
  ControllerServer.h
  ICamera.h
  CameraProxy.h
  CameraFile.h
  ConfigurationFile.h
  ICNC.h
  CNCProxy.cpp
  CNCProxy.h
  IImageCropper.h
  ImageCropperFactory.h
  ImageCropperFactory.cpp
  ImageCropper.h
  ImageCropper.cpp  
  IImageSegmentation.h
  ImageSegmentationFactory.h
  ImageSegmentationFactory.cpp
  svm/SVMSegmentation.h
  svm/SVMSegmentation.cpp
  quincunx/Quincunx.h
  quincunx/Quincunx.cpp
  quincunx/point.h
  quincunx/point.c
  som/SOM.cpp 
  som/Superpixels.cpp 
  som/slic.cpp 
  som/fixed.c 
  IPathPlanner.h
  PathPlannerFactory.h
  PathPlannerFactory.cpp
  IPipeline.h
  Pipeline.h
  Pipeline.cpp
  IFileCabinet.h
  DebugWeedingSession.h
  DebugWeedingSession.cpp
  Weeder.h
  Weeder.cpp
  weeder_main.cpp)

target_link_libraries(weeder2 rcompp romi)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

#ADD_CUSTOM_COMMAND(
#  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp
#  COMMAND rcgen code ${CMAKE_CURRENT_LIST_DIR}/weeder_main.cpp ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/rcgen.json
#  )

install(TARGETS weeder2 DESTINATION bin/romi)
