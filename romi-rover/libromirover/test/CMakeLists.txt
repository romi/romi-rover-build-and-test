
cmake_minimum_required(VERSION 3.10)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../include")

##

set(SRCS_V
  src/tests_main.cpp
  src/tests_v.cpp
  ../include/v.h
  ../src/v.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_v ${SRCS_V})
target_link_libraries(romi_rover_unit_tests_libromirover_v rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_v
    COMMAND romi_rover_unit_tests_libromirover_v
)

##

set(SRCS_CNCRANGE
  src/tests_main.cpp
  src/tests_cncrange.cpp
  ../include/CNCRange.h
  ../include/v.h
  ../src/CNCRange.cpp
  ../src/v.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_cncrange ${SRCS_CNCRANGE})
target_link_libraries(romi_rover_unit_tests_libromirover_cncrange rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_cncrange
    COMMAND romi_rover_unit_tests_libromirover_cncrange
)

##

set(SRCS_PATH
  src/tests_main.cpp
  src/tests_path.cpp
  ../include/CNCRange.h
  ../include/Path.h
  ../include/v.h
  ../src/CNCRange.cpp
  ../src/Path.cpp
  ../src/v.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_path ${SRCS_PATH})
target_link_libraries(romi_rover_unit_tests_libromirover_path rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_path
    COMMAND romi_rover_unit_tests_libromirover_path
)

##

set(SRCS_ROVERCONFIGURATION 
  src/tests_main.cpp
  src/tests_roverconfiguration.cpp
  ../include/RoverConfiguration.h
  )

add_executable(romi_rover_unit_tests_libromirover_roverconfiguration ${SRCS_ROVERCONFIGURATION})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_libromirover_roverconfiguration rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_roverconfiguration
    COMMAND romi_rover_unit_tests_libromirover_roverconfiguration
)


##

set(SRCS_WHEELODOMETRY 
  src/tests_main.cpp
  src/tests_wheelodometry.cpp
  ../include/WheelOdometry.h
  ../src/WheelOdometry.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_wheelodometry ${SRCS_WHEELODOMETRY})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_libromirover_wheelodometry rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_wheelodometry
    COMMAND romi_rover_unit_tests_libromirover_wheelodometry
)

##

set(SRCS_IMAGE
  src/tests_main.cpp
  src/tests_image.cpp
  ../include/Image.h
  ../src/Image.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_image ${SRCS_IMAGE})
target_link_libraries(romi_rover_unit_tests_libromirover_image rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_image
    COMMAND romi_rover_unit_tests_libromirover_image
)

##

set(SRCS_IMAGEIO
  src/tests_main.cpp
  src/tests_imageio.cpp
  ../include/Image.h
  ../include/ImageIO.h
  ../src/Image.cpp
  ../src/ImageIO.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_imageio ${SRCS_IMAGEIO})
target_link_libraries(romi_rover_unit_tests_libromirover_imageio rpp gtest gmock jpeg png)

add_test(
    NAME romi_rover_unit_tests_libromirover_imageio
    COMMAND romi_rover_unit_tests_libromirover_imageio
)

##

set(SRCS_ROVEROPTIONS
  src/tests_main.cpp
  src/tests_roveroptions.cpp
  ../include/GetOpt.h
  ../include/RoverOptions.h
  ../src/GetOpt.cpp
  ../src/rover/RoverOptions.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_roveroptions ${SRCS_ROVEROPTIONS})
target_link_libraries(romi_rover_unit_tests_libromirover_roveroptions rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_roveroptions
    COMMAND romi_rover_unit_tests_libromirover_roveroptions
)

##

set(SRCS_NAVIGATIONADAPTOR 
  src/tests_main.cpp
  src/tests_navigationadaptor.cpp
  mock/mock_navigation.h
  ../include/rpc/NavigationAdaptor.h
  ../src/rpc/NavigationAdaptor.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_navigationadaptor ${SRCS_NAVIGATIONADAPTOR})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_libromirover_navigationadaptor romirover rpp rcompp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_navigationadaptor
    COMMAND romi_rover_unit_tests_libromirover_navigationadaptor
)

##

set(SRCS_ROVERNAVIGATION 
  src/tests_main.cpp
  src/tests_rovernavigation.cpp
  mock/mock_motordriver.h
  ../include/RoverNavigation.h
  ../src/RoverNavigation.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_rovernavigation ${SRCS_ROVERNAVIGATION})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_libromirover_rovernavigation romirover rpp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_rovernavigation
    COMMAND romi_rover_unit_tests_libromirover_rovernavigation
)

##

set(SRCS_REMOTENAVIGATION 
  src/tests_main.cpp
  src/tests_remotenavigation.cpp
  ../include/rpc/RemoteNavigation.h
  ../src/rpc/RemoteNavigation.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_remotenavigation ${SRCS_REMOTENAVIGATION})
target_link_libraries(romi_rover_unit_tests_libromirover_remotenavigation rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_remotenavigation
    COMMAND romi_rover_unit_tests_libromirover_remotenavigation
)

##

set(SRCS_CRYSTALDISPLAY 
  src/tests_main.cpp
  src/tests_crystaldisplay.cpp
  mock/mock_romiserialclient.h
  ../include/CrystalDisplay.h
  ../src/ui/CrystalDisplay.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_crystaldisplay ${SRCS_CRYSTALDISPLAY})
target_link_libraries(romi_rover_unit_tests_libromirover_crystaldisplay rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_crystaldisplay
    COMMAND romi_rover_unit_tests_libromirover_crystaldisplay
)

##

set(SRCS_JOYSTICK 
  src/tests_main.cpp
  src/tests_joystick.cpp
  mock/Linux-Mock.cpp
  ../include/Joystick.h
  ../include/LinuxJoystick.h
  ../src/ui/LinuxJoystick.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_joystick ${SRCS_JOYSTICK})
target_link_libraries(romi_rover_unit_tests_libromirover_joystick rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_joystick
    COMMAND romi_rover_unit_tests_libromirover_joystick
)

##

set(SRCS_JOYSTICKINPUTDEVICE 
  src/tests_main.cpp
  src/tests_joystickinputdevice.cpp
  mock/mock_eventmapper.h
  mock/mock_joystick.h
  ../include/JoystickInputDevice.h
  ../src/ui/JoystickInputDevice.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_joystickinputdevice ${SRCS_JOYSTICKINPUTDEVICE})
target_link_libraries(romi_rover_unit_tests_libromirover_joystickinputdevice rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_joystickinputdevice
    COMMAND romi_rover_unit_tests_libromirover_joystickinputdevice
)

##

set(SRCS_SCRIPTLIST 
  src/tests_main.cpp
  src/tests_scriptlist.cpp
  mock/mock_inputdevice.h
  mock/mock_navigation.h
  mock/mock_display.h
  ../include/ScriptList.h
  ../src/ScriptList.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_scriptlist ${SRCS_SCRIPTLIST})
target_link_libraries(romi_rover_unit_tests_libromirover_scriptlist rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_scriptlist
    COMMAND romi_rover_unit_tests_libromirover_scriptlist
)

##

set(SRCS_ROVERSCRIPTENGINE 
  src/tests_main.cpp
  src/tests_roverscriptengine.cpp
  mock/mock_inputdevice.h
  mock/mock_navigation.h
  mock/mock_display.h
  ../include/Rover.h
  ../include/RoverScriptEngine.h
  ../src/rover/Rover.cpp
  ../src/rover/RoverScriptEngine.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_roverscriptengine ${SRCS_ROVERSCRIPTENGINE})
target_link_libraries(romi_rover_unit_tests_libromirover_roverscriptengine rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_roverscriptengine
    COMMAND romi_rover_unit_tests_libromirover_roverscriptengine
)

##

set(SRCS_STATEMACHINE 
  src/tests_main.cpp
  src/tests_statemachine.cpp
  ../include/StateMachine.h
  )

add_executable(romi_rover_unit_tests_libromirover_statemachine ${SRCS_STATEMACHINE})
target_link_libraries(romi_rover_unit_tests_libromirover_statemachine rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_statemachine
    COMMAND romi_rover_unit_tests_libromirover_statemachine
)

##

set(SRCS_SPEEDCONVERTER 
  src/tests_main.cpp
  src/tests_speedconverter.cpp
  mock/mock_navigation.h
  ../include/SpeedConverter.h
  ../src/SpeedConverter.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_speedconverter ${SRCS_SPEEDCONVERTER})
target_link_libraries(romi_rover_unit_tests_libromirover_speedconverter rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_speedconverter
    COMMAND romi_rover_unit_tests_libromirover_speedconverter
)

##

set(SRCS_SPEEDCONTROLLER 
  src/tests_main.cpp
  src/tests_speedcontroller.cpp
  mock/mock_navigation.h
  ../include/DefaultSpeedController.h
  ../src/DefaultSpeedController.cpp
  ../include/SpeedConverter.h
  ../src/SpeedConverter.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_speedcontroller ${SRCS_SPEEDCONTROLLER})
target_link_libraries(romi_rover_unit_tests_libromirover_speedcontroller rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_speedcontroller
    COMMAND romi_rover_unit_tests_libromirover_speedcontroller
)

##

set(SRCS_SCRIPTMENU 
  src/tests_main.cpp
  src/tests_scriptmenu.cpp
  mock/mock_inputdevice.h
  mock/mock_navigation.h
  mock/mock_display.h
  ../include/ScriptMenu.h
  ../src/ScriptMenu.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_scriptmenu ${SRCS_SCRIPTMENU})
target_link_libraries(romi_rover_unit_tests_libromirover_scriptmenu rcompp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_scriptmenu
    COMMAND romi_rover_unit_tests_libromirover_scriptmenu
)

##

set(SRCS_CNCADAPTOR
  src/tests_main.cpp
  src/tests_cncadaptor.cpp
  ../include/rpc/CNCAdaptor.h
  ../src/rpc/CNCAdaptor.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_cncadaptor ${SRCS_CNCADAPTOR})
target_link_libraries(romi_rover_unit_tests_libromirover_cncadaptor rpp romirover rcompp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_cncadaptor
    COMMAND romi_rover_unit_tests_libromirover_cncadaptor
)

##

set(SRCS_STEPPERSETTINGS
  src/tests_main.cpp
  src/tests_steppersettings.cpp
  ../include/oquam/StepperSettings.h
  ../src/oquam/StepperSettings.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_steppersettings ${SRCS_STEPPERSETTINGS})
target_link_libraries(romi_rover_unit_tests_libromirover_steppersettings rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_steppersettings
    COMMAND romi_rover_unit_tests_libromirover_steppersettings
)

##

set(SRCS_SECTION
  src/tests_main.cpp
  src/tests_oquam_section.cpp
  ../include/oquam/Section.h
  ../src/oquam/Section.cpp
  ../include/oquam/SmoothPath.h
  ../src/oquam/SmoothPath.cpp
  ../include/oquam/ATDC.h
  ../src/oquam/ATDC.cpp
  ../src/oquam/is_valid.cpp
  ../include/oquam/is_valid.h
  ../include/oquam/print.h
  ../src/oquam/print.cpp
  )
add_executable(romi_rover_unit_tests_libromirover_oquam_section ${SRCS_SECTION})
target_link_libraries(romi_rover_unit_tests_libromirover_oquam_section rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_oquam_section
    COMMAND romi_rover_unit_tests_libromirover_oquam_section
)

##

set(SRCS_PRINT
  src/tests_main.cpp
  src/tests_oquam_print.cpp
  ../include/oquam/SmoothPath.h
  ../src/oquam/SmoothPath.cpp
  ../include/oquam/ATDC.h
  ../src/oquam/ATDC.cpp
  ../include/oquam/Section.h
  ../src/oquam/Section.cpp
  ../include/oquam/print.h
  ../src/oquam/print.cpp
  ../src/oquam/is_valid.cpp
  ../include/oquam/is_valid.h
  )
add_executable(romi_rover_unit_tests_libromirover_oquam_print ${SRCS_PRINT})
target_link_libraries(romi_rover_unit_tests_libromirover_oquam_print rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_oquam_print
    COMMAND romi_rover_unit_tests_libromirover_oquam_print
)

##

set(SRCS_SCRIPT
  src/tests_main.cpp
  src/tests_oquam_script.cpp
  ../include/oquam/SmoothPath.h
  ../src/oquam/SmoothPath.cpp
  ../include/oquam/ATDC.h
  ../src/oquam/ATDC.cpp
  ../include/oquam/Section.h
  ../src/oquam/Section.cpp
  ../include/oquam/print.h
  ../src/oquam/print.cpp
  ../src/oquam/is_valid.cpp
  ../include/oquam/is_valid.h
  )
add_executable(romi_rover_unit_tests_libromirover_oquam_script ${SRCS_SCRIPT})
target_link_libraries(romi_rover_unit_tests_libromirover_oquam_script rpp romirover gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_oquam_script
    COMMAND romi_rover_unit_tests_libromirover_oquam_script
)

##

set(SRCS_OQUAM
  src/tests_main.cpp
  src/tests_oquam.cpp
  mock/mock_cnccontroller.h
  ../include/oquam/CNCController.h
  ../include/oquam/Oquam.h
  ../src/oquam/Oquam.cpp
  ../src/oquam/plotter.cpp
  ../include/oquam/plotter.h
  ../include/oquam/print.h
  ../src/oquam/print.cpp
  ../src/oquam/is_valid.cpp
  ../include/oquam/is_valid.h
  ../include/oquam/Section.h
  ../src/oquam/Section.cpp
  ../include/oquam/ATDC.h
  ../src/oquam/ATDC.cpp
  ../include/oquam/SmoothPath.h
  ../src/oquam/SmoothPath.cpp
  ../src/oquam/Oquam.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_oquam ${SRCS_OQUAM})
target_link_libraries(romi_rover_unit_tests_libromirover_oquam rpp romirover romi gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_oquam
    COMMAND romi_rover_unit_tests_libromirover_oquam
    )

##
  
set(SRCS_BRUSHMOTORDRIVER 
  src/tests_main.cpp
  src/tests_brushmotordriver.cpp
  )

add_executable(romi_rover_unit_tests_libromirover_brushmotordriver ${SRCS_BRUSHMOTORDRIVER})
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/../src" )
target_link_libraries(romi_rover_unit_tests_libromirover_brushmotordriver romirover romiserial rpp rcompp gtest gmock)

add_test(
    NAME romi_rover_unit_tests_libromirover_brushmotordriver
    COMMAND romi_rover_unit_tests_libromirover_brushmotordriver
)


##
  
if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_rover_libromirover_coverage
            EXECUTABLE ctest -V ${n_cores}
            DEPENDENCIES romi_rover_unit_tests_libromirover_v
                         romi_rover_unit_tests_libromirover_cncrange
                         romi_rover_unit_tests_libromirover_path
                         romi_rover_unit_tests_libromirover_image
                         romi_rover_unit_tests_libromirover_imageio
                         romi_rover_unit_tests_libromirover_wheelodometry
                         romi_rover_unit_tests_libromirover_roverconfiguration
                         romi_rover_unit_tests_libromirover_roveroptions
                         romi_rover_unit_tests_libromirover_navigationadaptor
                         romi_rover_unit_tests_libromirover_rovernavigation
                         romi_rover_unit_tests_libromirover_remotenavigation
                         romi_rover_unit_tests_libromirover_crystaldisplay
                         romi_rover_unit_tests_libromirover_joystick
                         romi_rover_unit_tests_libromirover_joystickinputdevice
                         romi_rover_unit_tests_libromirover_scriptlist
                         romi_rover_unit_tests_libromirover_scriptmenu
                         romi_rover_unit_tests_libromirover_roverscriptengine
                         romi_rover_unit_tests_libromirover_statemachine
                         romi_rover_unit_tests_libromirover_speedconverter
                         romi_rover_unit_tests_libromirover_speedcontroller
                         romi_rover_unit_tests_libromirover_scriptmenu
                         romi_rover_unit_tests_libromirover_cncadaptor
                         romi_rover_unit_tests_libromirover_steppersettings
                         romi_rover_unit_tests_libromirover_oquam_section
                         romi_rover_unit_tests_libromirover_oquam_script
                         romi_rover_unit_tests_libromirover_oquam_print
                         romi_rover_unit_tests_libromirover_oquam
                         romi_rover_unit_tests_libromirover_brushmotordriver)
endif()

