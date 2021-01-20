cmake_minimum_required(VERSION 2.6)
project(libromirover)

set(libromirover_VERSION_MAJOR 0)
set(libromirover_VERSION_MINOR 1)

  
set(SOURCES
  include/ActivityHelper.h
  include/BrushMotorDriver.h
  include/CameraServer.h
  include/CrystalDisplay.h
  include/DebugWeedingSession.h
  include/DefaultEventTimer.h
  include/DefaultNavigation.h
  include/DefaultSpeedController.h
  include/EventTimer.h
  include/FileCamera.h
  include/FluidSoundNotifications.h
  include/GetOpt.h
  include/IFileCabinet.h
  include/IFolder.h
  include/Image.h
  include/ImageIO.h
  include/JoystickInputDevice.h
  include/JoystickEventMapper.h
  include/LinuxJoystick.h
  include/Menu.h
  include/MotorDriver.h
  include/NavigationSettings.h
  include/Options.h
  include/ScriptEngine.h
  include/ScriptList.h
  include/ScriptMenu.h
  include/SpeedController.h
  include/SpeedConverter.h
  include/UIEventMapper.h
  include/USBCamera.h
  include/v.h
  include/WheelOdometry.h
  
  include/api/Activity.h
  include/api/Camera.h
  include/api/CNC.h
  include/api/CNCRange.h
  include/api/Display.h
  include/api/EventSource.h
  include/api/InputDevice.h
  include/api/Joystick.h
  include/api/Navigation.h
  include/api/Notifications.h
  include/api/Path.h
  include/api/StateMachine.h
  include/api/Weeder.h

  include/fake/FakeCNC.h
  include/fake/FakeDisplay.h
  include/fake/FakeInputDevice.h
  include/fake/FakeMotorDriver.h
  include/fake/FakeNavigation.h
  include/fake/FakeNotifications.h
  include/fake/FakeWeeder.h

  include/oquam/ATDC.h
  include/oquam/CNCController.h
  include/oquam/FakeCNCController.h
  include/oquam/is_valid.h
  include/oquam/Oquam.h
  include/oquam/plotter.h
  include/oquam/print.h
  include/oquam/SmoothPath.h
  include/oquam/Section.h
  include/oquam/StepperController.h
  include/oquam/StepperSettings.h

  include/rover/EventsAndStates.h
  include/rover/Rover.h
  include/rover/RoverInterface.h
  include/rover/RoverNotifications.h
  include/rover/RoverOptions.h
  include/rover/RoverScriptEngine.h
  include/rover/RoverStateMachine.h

  include/rpc/CNCAdaptor.h
  include/rpc/MethodsActivity.h
  include/rpc/MethodsCNC.h
  include/rpc/MethodsNavigation.h
  include/rpc/MethodsRover.h
  include/rpc/MethodsWeeder.h
  include/rpc/NavigationAdaptor.h
  include/rpc/RemoteCNC.h
  include/rpc/RemoteNavigation.h
  include/rpc/RemoteStub.h
  include/rpc/RemoteWeeder.h
  include/rpc/WeederAdaptor.h

  include/weeder/DefaultWeeder.h
  include/weeder/IImageCropper.h
  include/weeder/IImageSegmentation.h
  include/weeder/ImageCropper.h
  include/weeder/IPathPlanner.h
  include/weeder/IPipeline.h
  include/weeder/PipelineFactory.h
  include/weeder/Pipeline.h
  
  src/api/CNCRange.cpp
  src/api/Path.cpp
  
  src/ActivityHelper.cpp
  src/BrushMotorDriver.cpp
  src/DebugWeedingSession.cpp
  src/DefaultEventTimer.cpp
  src/DefaultNavigation.cpp
  src/DefaultSpeedController.cpp
  src/GetOpt.cpp
  src/Image.cpp
  src/ImageIO.cpp
  src/ScriptList.cpp
  src/ScriptMenu.cpp
  src/SpeedConverter.cpp
  src/v.cpp
  src/WheelOdometry.cpp
  src/camera/CameraServer.cpp
  src/camera/FileCamera.cpp
  src/camera/USBCamera.cpp
  src/camera/camera_v4l.h
  src/camera/camera_v4l.c
  src/notifications/FluidSoundNotifications.cpp
  src/oquam/ATDC.cpp
  src/oquam/is_valid.cpp
  src/oquam/Oquam.cpp
  src/oquam/plotter.cpp
  src/oquam/print.cpp
  src/oquam/SmoothPath.cpp
  src/oquam/Section.cpp
  src/oquam/StepperSettings.cpp
  src/oquam/StepperController.cpp
  
  src/rover/EventsAndStates.cpp
  src/rover/Rover.cpp
  src/rover/RoverInterface.cpp
  src/rover/RoverOptions.cpp
  src/rover/RoverScriptEngine.cpp
  src/rover/RoverStateMachine.cpp
  
  src/rpc/CNCAdaptor.cpp
  src/rpc/NavigationAdaptor.cpp
  src/rpc/RemoteCNC.cpp
  src/rpc/RemoteNavigation.cpp
  src/rpc/RemoteStub.cpp
  src/rpc/RemoteWeeder.cpp
  src/rpc/WeederAdaptor.cpp
  
  src/ui/CrystalDisplay.cpp
  src/ui/JoystickInputDevice.cpp
  src/ui/LinuxJoystick.cpp
  src/ui/UIEventMapper.cpp

  src/weeder/DefaultWeeder.cpp  
  src/weeder/ImageCropper.cpp
  src/weeder/Pipeline.cpp
  src/weeder/PipelineFactory.cpp
  src/weeder/quincunx/Quincunx.h
  src/weeder/quincunx/Quincunx.cpp
  src/weeder/quincunx/point.h
  src/weeder/quincunx/point.c
  src/weeder/svm/SVMSegmentation.h
  src/weeder/svm/SVMSegmentation.cpp
  src/weeder/som/SOM.cpp 
  src/weeder/som/Superpixels.cpp 
  src/weeder/som/slic.cpp 
  src/weeder/som/fixed.c 
  )


include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )

set_source_files_properties(src/weeder/som/SOM.cpp PROPERTIES COMPILE_FLAGS -O3)

add_library(romirover SHARED ${SOURCES})


target_include_directories(romirover
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

target_link_libraries(romirover
                        romiserial
                        rpp
                        rcompp
                        fluidsynth
                        jpeg
                        png
                        m)
                      
if(BUILD_TESTS)
    add_subdirectory(test)
endif()

#INSTALL(TARGETS romi DESTINATION "lib")
#INSTALL (
#    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
#    DESTINATION include
#    FILES_MATCHING PATTERN "*.h")
