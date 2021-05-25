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
#include <Clock.h>
#include <ClockAccessor.h>
#include <configuration/ConfigurationProvider.h>
#include <rover/Rover.h>
#include <rover/RoverOptions.h>
#include <rover/RoverInterface.h>
#include <rover/RoverScriptEngine.h>
#include <rover/RoverStateMachine.h>
#include <rover/SpeedController.h>
#include <api/EventTimer.h>
#include <ui/ScriptList.h>
#include <ui/ScriptMenu.h>
#include <notifications/FluidSoundNotifications.h>
#include <rover/Navigation.h>
#include <ui/CrystalDisplay.h>
#include <ui/JoystickInputDevice.h>
#include <weeder/Weeder.h>
#include <api/IDisplay.h>
#include <ui/LinuxJoystick.h>
#include <ui/UIEventMapper.h>
#include <ui/CrystalDisplay.h>
#include <ui/JoystickInputDevice.h>
#include <camera/FileCamera.h>
#include <camera/USBCamera.h>
#include <rpc/RemoteCamera.h>
#include <rpc/RcomClient.h>
#include <hal/BrushMotorDriver.h>
#include <oquam/StepperController.h>
#include <oquam/StepperSettings.h>
#include <oquam/Oquam.h>
#include <weeder/PipelineFactory.h>
#include <camera/Imager.h>
#include <unet/UnetImager.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>

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
                
                rpp::Linux linux;

                // Session
                r_info("main: Creating session");
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = romi::get_session_directory(options, config);

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));
                session.start("hw_observation_id");

                // Display
                r_info("main: Creating display");
                const char *display_device = (const char *) config["ports"]["display-device"]["port"];
                
                auto display_serial = romiserial::RomiSerialClient::create(display_device);
                romi::CrystalDisplay display(display_serial);
                display.show(0, "Initializing");

                // Joystick
                r_info("main: Creating joystick");
                const char *joystick_device = (const char *) config["ports"]["joystick"]["port"];
                romi::LinuxJoystick joystick(linux, joystick_device);
                romi::UIEventMapper joystick_event_mapper;
                romi::JoystickInputDevice input_device(joystick, joystick_event_mapper);

                // CNC controller
                r_info("main: Creating CNC controller");
                const char *cnc_device = (const char *) config["ports"]["oquam"]["port"];
                auto cnc_serial = romiserial::RomiSerialClient::create(cnc_device);
                romi::StepperController cnc_controller(cnc_serial);

                
                // CNC
                r_info("main: Creating CNC");
                JsonCpp r = config["oquam"]["cnc-range"];
                romi::CNCRange range(r);
                JsonCpp s = config["oquam"]["stepper-settings"];
                romi::StepperSettings stepper_settings(s);
                double slice_duration = (double) config["oquam"]["path-slice-duration"];
                double maximum_deviation = (double) config["oquam"]["path-maximum-deviation"];

                romi::AxisIndex homing[3] = { romi::kAxisX, romi::kAxisY, romi::kNoAxis };
                //romi::AxisIndex homing[3] = { romi::kNoAxis, romi::kNoAxis, romi::kNoAxis };
                romi::OquamSettings oquam_settings(range,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   homing);
                romi::Oquam oquam(cnc_controller, oquam_settings, session);

                // Camera
                // TBD: Use refactored functions. get_camera_class
                r_info("main: Creating camera");
                std::unique_ptr<romi::ICamera> camera;
                std::string camera_classname = get_camera_classname(options, config);
                
                if (camera_classname == romi::FileCamera::ClassName) {
                        std::string image_file = get_camera_image(options, config);
                        r_info("Loading image %s", image_file.c_str());
                        camera = std::make_unique<romi::FileCamera>(image_file);
                        
                } else if (camera_classname == romi::USBCamera::ClassName) {
                        std::string camera_device = get_camera_device(options, config);
                        double width = (double) config["weeder"]["usb-camera"]["width"];
                        double height = (double) config["weeder"]["usb-camera"]["height"];
                        camera = std::make_unique<romi::USBCamera>(camera_device, width, height);
                } else if (camera_classname == romi::RemoteCamera::ClassName) {
                        auto client = romi::RcomClient::create("camera", 10.0);
                        camera = std::make_unique<romi::RemoteCamera>(client);
                } else {
                        throw std::runtime_error("Unknown camera classname");
                }
                
                // Weeder pipeline
                r_info("main: Creating weeder pipeline");
                romi::PipelineFactory pipeline_factory;
                romi::IPipeline& pipeline = pipeline_factory.build(range, config, options);

                // Weeder
                r_info("main: Creating weeder");
                double z0 = (double) config["weeder"]["z0"];
                double speed = (double) config["weeder"]["speed"];
                romi::Weeder weeder(*camera, pipeline, oquam, z0, speed, session);

                // Motor driver
                r_info("main: Creating motor driver");
                JsonCpp rover_settings = config["navigation"]["rover"];
                romi::NavigationSettings rover_config(rover_settings);
                const char *driver_device = (const char *) config["ports"]["brush-motor-driver"]["port"];
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                auto driver_serial = romiserial::RomiSerialClient::create(driver_device);
                romi::BrushMotorDriver driver(driver_serial, driver_settings,
                                              static_cast<int>(rover_config.encoder_steps),
                                              rover_config.max_revolutions_per_sec);
                
                // Navigation
                r_info("main: Creating navigation");
                romi::Navigation navigation(driver, rover_config);

                // SpeedController
                r_info("main: Creating speed controller");
                romi::SpeedController speed_controller(navigation, config);

                // EventTimer
                r_info("main: Creating event timer");
                romi::EventTimer event_timer(romi::event_timer_timeout);

                // Scripts and script engine
                std::string script_file = get_script_file(options, config);
                r_info("main: Loading scripts from %s", script_file.c_str());
                romi::ScriptList scripts(script_file);
                
                r_info("main: Creating scripts engine");
                romi::ScriptMenu menu(scripts);
                romi::RoverScriptEngine script_engine(scripts, romi::event_script_finished,
                                                      romi::event_script_error);

                // Notifications
                std::string soundfont = get_sound_font_file(options, config);
                r_info("main: Loading soundfont %s", soundfont.c_str());
                JsonCpp sound_setup = config["user-interface"]["fluid-sounds"]["sounds"];
                r_info("main: Creating sound notifications");
                romi::FluidSoundNotifications notifications(soundfont, sound_setup);

                // Imager
                //romi::Imager imager(session, *camera);
                romi::UnetImager imager(session, *camera);

        
                // Rover
                r_info("main: Creating rover");
                romi::Rover rover(input_device,
                                  display,
                                  speed_controller,
                                  navigation,
                                  event_timer,
                                  menu,
                                  script_engine,
                                  notifications,
                                  weeder,
                                  imager);

                // State machine
                r_info("main: Creating state machine");
                romi::RoverStateMachine state_machine(rover);

                // User interface
                r_info("main: Creating user interface");
                romi::RoverInterface user_interface(rover, state_machine);

                r_info("main: Sending start event");
                
                if (!state_machine.handle_event(romi::event_start))
                        // FIXME: should not quit but display something
                        throw std::runtime_error("start-up failed");
                
                r_info("main: Starting event loop");
                
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
