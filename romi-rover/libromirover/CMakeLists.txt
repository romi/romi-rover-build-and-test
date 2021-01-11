cmake_minimum_required(VERSION 2.6)
project(libromirover)

set(libromirover_VERSION_MAJOR 0)
set(libromirover_VERSION_MINOR 1)

  
set(SOURCES
  include/Activity.h
  include/ActivityHelper.h
  include/BrushMotorDriver.h
  include/Camera.h
  include/CameraServer.h
  include/CNC.h
  include/CNCRange.h
  include/CrystalDisplay.h
  include/DebugWeedingSession.h
  include/DefaultEventTimer.h
  include/DefaultSpeedController.h
  include/Display.h
  include/EventsAndStates.h
  include/EventSource.h
  include/EventTimer.h
  include/FakeCNC.h
  include/FakeDisplay.h
  include/FakeInputDevice.h
  include/FakeMotorDriver.h
  include/FakeNavigation.h
  include/FakeNotifications.h
  include/FakeWeeder.h
  include/FileCamera.h
  include/FluidSoundNotifications.h
  include/GetOpt.h
  include/IFileCabinet.h
  include/IFolder.h
  include/Image.h
  include/ImageIO.h
  include/InputDevice.h
  include/Joystick.h
  include/JoystickInputDevice.h
  include/JoystickEventMapper.h
  include/LinuxJoystick.h
  include/Menu.h
  include/MotorDriver.h
  include/Navigation.h
  include/Notifications.h
  include/Options.h
  include/Path.h
  include/Rover.h
  include/RoverConfiguration.h
  include/RoverNavigation.h
  include/RoverOptions.h
  include/RoverScriptEngine.h
  include/RoverStateMachine.h
  include/RoverWeeder.h
  include/ScriptEngine.h
  include/ScriptList.h
  include/ScriptMenu.h
  include/SpeedController.h
  include/SpeedConverter.h
  include/StateMachine.h
  include/UIEventMapper.h
  include/USBCamera.h
  include/UserInterface.h
  include/v.h
  include/Weeder.h
  include/WheelOdometry.h
  
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

  src/ActivityHelper.cpp
  src/BrushMotorDriver.cpp
  src/CNCRange.cpp
  src/DebugWeedingSession.cpp
  src/DefaultEventTimer.cpp
  src/DefaultSpeedController.cpp
  src/EventsAndStates.cpp
  src/GetOpt.cpp
  src/Image.cpp
  src/ImageIO.cpp
  src/Path.cpp
  src/RoverNavigation.cpp
  src/RoverWeeder.cpp
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
  src/rover/Rover.cpp
  src/rover/RoverOptions.cpp
  src/rover/RoverScriptEngine.cpp
  src/rover/RoverStateMachine.cpp
  src/rover/UserInterface.cpp
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
  
  include/weeder/IImageCropper.h
  include/weeder/IImageSegmentation.h
  include/weeder/IPipeline.h  
  include/weeder/PipelineFactory.h
  include/weeder/ImageCropper.h
  include/weeder/IPathPlanner.h
  include/weeder/Pipeline.h
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
