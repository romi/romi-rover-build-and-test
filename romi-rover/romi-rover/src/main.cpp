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
#include <atomic>
#include <syslog.h>

#include <string.h>
#include <RSerial.h>
#include <RomiSerialClient.h>

#include <Linux.h>
#include "configuration/ConfigurationProvider.h"
#include <rover/Rover.h>
#include <rover/RoverOptions.h>
#include <rover/RoverInterface.h>
#include <rover/RoverScriptEngine.h>
#include <rover/RoverStateMachine.h>
#include <SpeedController.h>
#include <EventTimer.h>
#include <ScriptList.h>
#include <ScriptMenu.h>
#include <FluidSoundNotifications.h>
#include <Navigation.h>
#include <CrystalDisplay.h>
#include <JoystickInputDevice.h>
#include <weeder/Weeder.h>
#include <api/IDisplay.h>
#include <LinuxJoystick.h>
#include <UIEventMapper.h>
#include <CrystalDisplay.h>
#include <JoystickInputDevice.h>
#include <FileCamera.h>
#include <USBCamera.h>
#include <BrushMotorDriver.h>
#include <oquam/StepperController.h>
#include <oquam/StepperSettings.h>
#include <oquam/Oquam.h>
#include <weeder/PipelineFactory.h>

#include "Clock.h"
#include "ClockAccessor.h"
#include "data_provider/RomiDeviceData.h"
#include "data_provider/SoftwareVersion.h"
#include "weeder/Session.h"
#include "data_provider/Gps.h"
#include "data_provider/GpsLocationProvider.h"

std::atomic<bool> quit(false);

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "rcom-registry segmentation fault");
                exit(signal);
        }
        else if (signal == SIGINT){
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        }
        else{
                r_err("Unknown signam received %d", signal);
        }
}

int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();

        r_log_init();
        r_log_set_app("romi-rover");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        // TBD: Check with Peter.
//        app_init(&argc, argv);
//        app_set_name("romi-rover");

        try {
                std::string config_file = options.get_config_file();
                r_info("Romi Rover: Using configuration file: '%s'", config_file.c_str());
                JsonCpp config = JsonCpp::load(config_file.c_str());

                // Display
                const char *display_device = (const char *) config["ports"]["crystal-display"]["port"];
                std::shared_ptr<RSerial>display_serial = std::make_shared<RSerial>(display_device, 115200, 1);
                RomiSerialClient display_romiserial(display_serial, display_serial);
                romi::CrystalDisplay display(display_romiserial);
                display.show(0, "Initializing");

                // Joystick
                rpp::Linux linux;
                const char *joystick_device = (const char *) config["ports"]["joystick"]["port"];
                romi::LinuxJoystick joystick(linux, joystick_device);
                romi::UIEventMapper joystick_event_mapper;
                romi::JoystickInputDevice input_device(joystick, joystick_event_mapper);

                // CNC controller
                const char *cnc_device = (const char *) config["ports"]["oquam"]["port"];
                std::shared_ptr<RSerial>cnc_serial = std::make_shared<RSerial>(cnc_device, 115200, 1);
                RomiSerialClient cnc_romiserial(cnc_serial, cnc_serial);
                romi::StepperController cnc_controller(cnc_romiserial);

                // CNC
                JsonCpp r = config["oquam"]["cnc-range"];
                romi::CNCRange range(r);
                JsonCpp s = config["oquam"]["stepper-settings"];
                romi::StepperSettings stepper_settings(s);
                double slice_duration = (double) config["oquam"]["path-slice-duration"];
                double maximum_deviation = (double) config["oquam"]["path-maximum-deviation"];

                // Session
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationPrivider = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = romi::get_session_directory(options, config);

                romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
                session.start("hw_observation_id");
                romi::Oquam oquam(cnc_controller, range,
                            stepper_settings.maximum_speed,
                            stepper_settings.maximum_acceleration,
                            stepper_settings.steps_per_meter,
                            maximum_deviation,
                            slice_duration,
                            session);

                // Camera
                // TBD: Use refactored functions. get_camera_class
                std::unique_ptr<romi::ICamera> camera;
                const char *camera_classname = (const char *) config["weeder"]["camera-classname"];
                if (rstreq(camera_classname, romi::FileCamera::ClassName)) {
                        std::string image_file = get_camera_image(options, config);
                        r_info("Loading image %s", image_file.c_str());
                        camera = std::make_unique<romi::FileCamera>(image_file);
                        
                } else {
                        std::string camera_device = get_camera_device(options, config);
                        double width = (double) config["weeder"]["usb-camera"]["width"];
                        double height = (double) config["weeder"]["usb-camera"]["height"];
                        camera = std::make_unique<romi::USBCamera>(camera_device, width, height);
                }
                
                
                // Weeder pipeline
                romi::PipelineFactory pipeline_factory;
                romi::IPipeline& pipeline = pipeline_factory.build(range, config);

                // Weeder
                double z0 = (double) config["weeder"]["z0"];
                double speed = (double) config["weeder"]["speed"];
                romi::Weeder weeder(*camera, pipeline, oquam, z0, speed, session);

                // Navigation
                JsonCpp rover_settings = config["navigation"]["rover"];
                romi::NavigationSettings rover_config(rover_settings);
                const char *driver_device = (const char *) config["ports"]["brush-motor-driver"]["port"];
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                std::shared_ptr<RSerial>driver_serial = std::make_shared<RSerial>(driver_device, 115200, 1);
                RomiSerialClient driver_romiserial(driver_serial, driver_serial);
                romi::BrushMotorDriver driver(driver_romiserial, driver_settings,
                                        static_cast<int>(rover_config.encoder_steps),
                                        rover_config.max_revolutions_per_sec);
                romi::Navigation navigation(driver, rover_config);

                // SpeedController
                romi::SpeedController speed_controller(navigation, config);

                // EventTimer
                romi::EventTimer event_timer(romi::event_timer_timeout);

                // Scripts and script engine
                romi::ScriptList scripts(get_script_file(options, config));
                romi::ScriptMenu menu(scripts);
                romi::RoverScriptEngine script_engine(scripts, romi::event_script_finished,
                                                      romi::event_script_error);

                // Notifications
                std::string soundfont = get_sound_font_file(options, config);
                JsonCpp sound_setup = config["user-interface"]["fluid-sounds"]["sounds"];
                romi::FluidSoundNotifications notifications(soundfont, sound_setup);

                // Rover
                romi::Rover rover(input_device,
                            display,
                            speed_controller,
                            navigation,
                            event_timer,
                            menu,
                            script_engine,
                            notifications,
                            weeder);

                // State machine
                romi::RoverStateMachine state_machine(rover);

                // User interface
                romi::RoverInterface user_interface(rover, state_machine);

                
                if (!state_machine.handle_event(romi::event_start))
                        // FIXME: should not quit but display something
                        throw std::runtime_error("start-up failed");
                
                while (!quit) {
                        
                        try {
                                user_interface.handle_events();
                        
                        } catch (std::exception& e) {
                                
                                navigation.stop();
                                throw;
                        }
                }

                retval = 0;

                
        } catch (JSONError& je) {
                r_err("main: Failed to read the configuration file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
