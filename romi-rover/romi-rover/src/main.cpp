/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <exception>
#include <stdexcept>
#include <memory>
#include <string.h>
#include <rcom.h>

#include <RSerial.h>
#include <RomiSerialClient.h>

#include "Linux.h"
#include "RoverOptions.h"
#include "DefaultSpeedController.h"
#include "UserInterface.h"
#include "DefaultEventTimer.h"
#include "ScriptList.h"
#include "ScriptMenu.h"
#include "FluidSoundNotifications.h"
#include "RoverScriptEngine.h"
#include "RoverStateMachine.h"
#include "Rover.h"
#include "RoverNavigation.h"
#include "CrystalDisplay.h"
#include "JoystickInputDevice.h"
#include "RoverWeeder.h"
#include "Display.h"
#include "LinuxJoystick.h"
#include "UIEventMapper.h"
#include "CrystalDisplay.h"
#include "JoystickInputDevice.h"
#include "DebugWeedingSession.h"
#include "FileCamera.h"
#include "USBCamera.h"
#include "BrushMotorDriver.h"
#include "oquam/StepperController.h"
#include "oquam/StepperSettings.h"
#include "oquam/Oquam.h"
#include "weeder/PipelineFactory.h"

using namespace std;
using namespace rpp;
using namespace rcom;
using namespace romi;


const char *get_sound_font_file(Options& options, JsonCpp& config)
{
        const char *file = options.get_value(soundfont_option);
        if (file == 0)
                file = config["user-interface"]["fluid-sounds"]["soundfont"];
        return file;
}

const char *get_script_file(Options& options, JsonCpp& config)
{
        const char *file = options.get_value(script_option);
        if (file == 0)
                file = config["user-interface"]["script-engine"]["script"];
        return file;
}



int main(int argc, char** argv)
{
        int retval = 1;
        
        GetOpt options(rover_options, rover_options_length);
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        app_set_name("romi-rover");

        try {
                r_debug("Romi Rover: Using configuration file: '%s'",
                        options.get_value(config_option));

                JsonCpp config = JsonCpp::load(options.get_value(config_option));

                // Display
                const char *display_device = config["ports"]["crystal-display"]["port"];
                RSerial display_serial(display_device, 115200, 1);
                RomiSerialClient display_romiserial(&display_serial, &display_serial);
                CrystalDisplay display(display_romiserial);
                display.show(0, "Initializing");

                // Joystick
                Linux linux;
                const char *joystick_device = config["ports"]["joystick"]["port"];
                LinuxJoystick joystick(linux, joystick_device);
                UIEventMapper joystick_event_mapper;
                JoystickInputDevice input_device(joystick, joystick_event_mapper);

                // CNC controller
                const char *cnc_device = config["ports"]["oquam"]["port"];
                RSerial cnc_serial(cnc_device, 115200, 1);
                RomiSerialClient cnc_romiserial(&cnc_serial, &cnc_serial);
                StepperController cnc_controller(cnc_romiserial);

                // CNC
                JsonCpp r = config["oquam"]["cnc-range"];
                CNCRange range(r);
                JsonCpp s = config["oquam"]["stepper-settings"];
                StepperSettings stepper_settings(s);        
                double slice_duration = config["oquam"]["path-slice-duration"];
                double maximum_deviation = config["oquam"]["path-maximum-deviation"];
                
                Oquam oquam(cnc_controller, range,
                            stepper_settings.maximum_speed,
                            stepper_settings.maximum_acceleration,
                            stepper_settings.steps_per_meter,
                            maximum_deviation,
                            slice_duration);
                
                DebugWeedingSession debug(options.get_value(session_directory_option),
                                          "oquam");
                oquam.set_file_cabinet(&debug);

                // Camera
                unique_ptr<Camera> camera;
                const char *camera_classname = config["weeder"]["camera-classname"];
                if (rstreq(camera_classname, FileCamera::ClassName)) {
                        const char *image_file = options.get_value(camera_image_option);
                        if (image_file == 0)
                                throw std::runtime_error("Missing image file for "
                                                         "file camera");
                        r_info("Loading image %s", image_file);
                        camera = make_unique<FileCamera>(image_file);
                        
                } else {
                        const char *camera_device = config["ports"]["usb-camera"]["port"];
                        double width = config["weeder"]["usb-camera"]["width"];
                        double height = config["weeder"]["usb-camera"]["height"];
                        camera = make_unique<USBCamera>(camera_device, width, height);
                }
                
                
                // Weeder pipeline
                PipelineFactory pipeline_factory;
                IPipeline& pipeline = pipeline_factory.build(range, config);

                // Weeder
                double z0 = config["weeder"]["z0"];
                double speed = config["weeder"]["speed"];
                RoverWeeder weeder(*camera, pipeline, oquam, z0, speed, debug);

                // Navigation
                JsonCpp rover_settings = config["navigation"]["rover"];
                RoverConfiguration rover_config(rover_settings);
                const char *driver_device = config["ports"]["brush-motor-driver"]["port"];
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                RSerial driver_serial(driver_device, 115200, 1);
                RomiSerialClient driver_romiserial(&driver_serial, &driver_serial);
                BrushMotorDriver driver(driver_romiserial, driver_settings,
                                        rover_config.encoder_steps,
                                        rover_config.max_revolutions_per_sec);
                RoverNavigation navigation(driver, rover_config);

                // SpeedController
                DefaultSpeedController speed_controller(navigation, config);

                // EventTimer
                DefaultEventTimer event_timer(event_timer_timeout);

                // Scripts and script engine
                ScriptList scripts(get_script_file(options, config));
                ScriptMenu menu(scripts);
                RoverScriptEngine script_engine(scripts, event_script_finished,
                                                event_script_error);

                // Notifications
                const char *soundfont = get_sound_font_file(options, config);
                JsonCpp sound_setup = config["user-interface"]["fluid-sounds"]["sounds"];
                FluidSoundNotifications notifications(soundfont, sound_setup);

                // Rover
                Rover rover(input_device,
                            display,
                            speed_controller,
                            navigation,
                            event_timer,
                            menu,
                            script_engine,
                            notifications,
                            weeder);

                // State machine
                RoverStateMachine state_machine(rover);

                // User interface
                UserInterface user_interface(rover, state_machine);

                
                if (!state_machine.handle_event(event_start))
                        // FIXME: should not quit but display something
                        throw std::runtime_error("Start-up failed"); 
                
                while (!app_quit()) {
                        
                        try {
                                user_interface.handle_events();
                        
                        } catch (exception& e) {
                                
                                navigation.stop();
                                throw e;
                        }
                }

                retval = 0;

                
        } catch (JSONError& je) {
                r_err("main: Failed to read the configuration file: %s", je.what());
                
        } catch (exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
