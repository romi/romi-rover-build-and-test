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

#include <Linux.h>
#include <rover/Rover.h>
#include <rover/RoverOptions.h>
#include <rover/RoverInterface.h>
#include <rover/RoverScriptEngine.h>
#include <rover/RoverStateMachine.h>
#include <DefaultSpeedController.h>
#include <DefaultEventTimer.h>
#include <ScriptList.h>
#include <ScriptMenu.h>
#include <FluidSoundNotifications.h>
#include <DefaultNavigation.h>
#include <CrystalDisplay.h>
#include <JoystickInputDevice.h>
#include <weeder/DefaultWeeder.h>
#include <api/Display.h>
#include <LinuxJoystick.h>
#include <UIEventMapper.h>
#include <CrystalDisplay.h>
#include <JoystickInputDevice.h>
#include <DebugWeedingSession.h>
#include <FileCamera.h>
#include <USBCamera.h>
#include <BrushMotorDriver.h>
#include <oquam/StepperController.h>
#include <oquam/StepperSettings.h>
#include <oquam/Oquam.h>
#include <weeder/PipelineFactory.h>

using namespace std;
using namespace rpp;
using namespace rcom;
using namespace romi;


// TBD: Duplicated functions here and in other main.cpp files.
const char *get_config_file(Options& options)
{
        const char *file = options.get_value(RoverOptions::config);
        if (file == nullptr) {
                throw std::runtime_error("No configuration file was given (can't run without one...).");
        }
        return file;
}

const char *get_script_file(Options& options, JsonCpp& config)
{
        const char *file = options.get_value(RoverOptions::script);
        if (file == nullptr) {
                file = (const char *) config["user-interface"]["script-engine"]["script-file"];
        }
        return file;
}

const char *get_sound_font_file(Options& options, JsonCpp& config)
{
        const char *file = options.get_value(RoverOptions::soundfont);
        if (file == nullptr)
                file = (const char *) config["user-interface"]["fluid-sounds"]["soundfont"];
        return file;
}

const char *get_session_directory(Options& options, __attribute__((unused))JsonCpp& config)
{
        const char *dir = options.get_value(RoverOptions::session_directory);
        if (dir == nullptr) {
                // TODO: to be finalized: get seesion dir from config
                // file, add the current date to the path, and create
                // a new directory.
                dir = ".";
        }
        return dir;
}

const char *get_camera_image(Options& options, JsonCpp& config)
{
        const char *path = options.get_value(RoverOptions::camera_image);
        if (path == nullptr) {
                path = (const char *) config["weeder"]["file-camera"]["image"];
        }
        return path;
}

const char *get_camera_device_in_config(JsonCpp& config)
{
        try {
                return (const char *) config["ports"]["usb-camera"]["port"];
                
        } catch (JSONError& je) {
                r_err("get_camera_device_in_config: Failed to get value "
                      "of ports.usb-camera.port");
                throw std::runtime_error("Missing device name for camera in config");
        }
}

const char *get_camera_device(Options& options, JsonCpp& config)
{
        const char *device = options.get_value(RoverOptions::camera_device);
        if (device == nullptr)
                device = get_camera_device_in_config(config);
        return device;
}

int main(int argc, char** argv)
{
        int retval = 1;
        
        RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();
        
        
        app_init(&argc, argv);
        app_set_name("romi-rover");

        try {
                const char *config_file = get_config_file(options);
                r_info("Romi Rover: Using configuration file: '%s'", config_file);
                JsonCpp config = JsonCpp::load(config_file);

                // Display
                const char *display_device = (const char *) config["ports"]["crystal-display"]["port"];
                std::shared_ptr<RSerial>display_serial = std::make_shared<RSerial>(display_device, 115200, 1);
                RomiSerialClient display_romiserial(display_serial, display_serial);
                CrystalDisplay display(display_romiserial);
                display.show(0, "Initializing");

                // Joystick
                Linux linux;
                const char *joystick_device = (const char *) config["ports"]["joystick"]["port"];
                LinuxJoystick joystick(linux, joystick_device);
                UIEventMapper joystick_event_mapper;
                JoystickInputDevice input_device(joystick, joystick_event_mapper);

                // CNC controller
                const char *cnc_device = (const char *) config["ports"]["oquam"]["port"];
                std::shared_ptr<RSerial>cnc_serial = std::make_shared<RSerial>(cnc_device, 115200, 1);
                RomiSerialClient cnc_romiserial(cnc_serial, cnc_serial);
                StepperController cnc_controller(cnc_romiserial);

                // CNC
                JsonCpp r = config["oquam"]["cnc-range"];
                CNCRange range(r);
                JsonCpp s = config["oquam"]["stepper-settings"];
                StepperSettings stepper_settings(s);        
                double slice_duration = (double) config["oquam"]["path-slice-duration"];
                double maximum_deviation = (double) config["oquam"]["path-maximum-deviation"];
                
                Oquam oquam(cnc_controller, range,
                            stepper_settings.maximum_speed,
                            stepper_settings.maximum_acceleration,
                            stepper_settings.steps_per_meter,
                            maximum_deviation,
                            slice_duration);
                
                DebugWeedingSession debug(get_session_directory(options, config),
                                          "romi-rover");
                oquam.set_file_cabinet(&debug);

                // Camera
                unique_ptr<Camera> camera;
                const char *camera_classname = (const char *) config["weeder"]["camera-classname"];
                if (rstreq(camera_classname, FileCamera::ClassName)) {
                        const char *image_file = get_camera_image(options, config);
                        r_info("Loading image %s", image_file);
                        camera = make_unique<FileCamera>(image_file);
                        
                } else {
                        const char *camera_device = get_camera_device(options, config);
                        double width = (double) config["weeder"]["usb-camera"]["width"];
                        double height = (double) config["weeder"]["usb-camera"]["height"];
                        camera = make_unique<USBCamera>(camera_device, width, height);
                }
                
                
                // Weeder pipeline
                PipelineFactory pipeline_factory;
                IPipeline& pipeline = pipeline_factory.build(range, config);

                // Weeder
                double z0 = (double) config["weeder"]["z0"];
                double speed = (double) config["weeder"]["speed"];
                DefaultWeeder weeder(*camera, pipeline, oquam, z0, speed, debug);

                // Navigation
                JsonCpp rover_settings = config["navigation"]["rover"];
                NavigationSettings rover_config(rover_settings);
                const char *driver_device = (const char *) config["ports"]["brush-motor-driver"]["port"];
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                std::shared_ptr<RSerial>driver_serial = std::make_shared<RSerial>(driver_device, 115200, 1);
                RomiSerialClient driver_romiserial(driver_serial, driver_serial);
                BrushMotorDriver driver(driver_romiserial, driver_settings,
                                        static_cast<int>(rover_config.encoder_steps),
                                        rover_config.max_revolutions_per_sec);
                DefaultNavigation navigation(driver, rover_config);

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
                RoverInterface user_interface(rover, state_machine);

                
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
