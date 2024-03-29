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
#include <atomic>
#include <syslog.h>

#include <rcom/Linux.h>

#include <RSerial.h>
#include <RomiSerialClient.h>

#include "configuration/ConfigurationProvider.h"
#include <util/RomiSerialLog.h>
#include <rover/Rover.h>
#include <rover/RoverOptions.h>
#include <rover/RoverInterface.h>
#include <rover/RoverStateMachine.h>
#include <rover/SpeedController.h>
#include <api/EventTimer.h>
#include <ui/ScriptList.h>
#include <ui/ScriptMenu.h>
#include <rover/Navigation.h>
#include <rover/ZeroNavigationController.h>
#include <rover/LocationTracker.h>
#include <ui/JoystickInputDevice.h>
#include <fake/FakeWeeder.h>
#include <fake/FakeScriptEngine.h>
#include <fake/FakeNotifications.h>
#include <api/IDisplay.h>
#include <ui/LinuxJoystick.h>
#include <ui/UIEventMapper.h>
#include <ui/CrystalDisplay.h>
#include <ui/JoystickInputDevice.h>
#include <ui/RemoteStateInputDevice.h>
#include <hal/BrushMotorDriver.h>
#include <camera/FakeImager.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <rover/WheelOdometry.h>
#include <rover/DifferentialSteering.h>
#include <util/Clock.h>
#include <util/ClockAccessor.h>

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
        std::shared_ptr<romi::IClock> clock = std::make_shared<romi::Clock>();
        romi::ClockAccessor::SetInstance(clock);

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();

        log_init();
        log_set_application("joystick-test");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);

        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());

                // TBD: USE FileUtils
                std::ifstream ifs(config_file);
                nlohmann::json config = nlohmann::json::parse(ifs);

                rcom::Linux linux;

                // Session
                romi::RomiDeviceData romiDeviceData("Joystick", "NA");
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationPrivider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = romi::get_session_directory(options, config);

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationPrivider));
                session.start("hw_observation_id");

                // Display
                std::string display_device = config["ports"]["display-device"]["port"];

                std::string client_name("display_device");
                std::shared_ptr<romiserial::ILog> log
                        = std::make_shared<romi::RomiSerialLog>();
                auto display_serial = romiserial::RomiSerialClient::create(display_device,
                                                                           client_name,
                                                                           log);
                romi::CrystalDisplay display(display_serial);
                display.clear(display.count_lines());
                display.show(0, "Initializing");
                
                // Joystick
                std::string joystick_device = config["ports"]["joystick"]["port"];
                romi::LinuxJoystick joystick(linux, joystick_device);
                romi::UIEventMapper joystick_event_mapper;
                romi::JoystickInputDevice input_device(joystick, joystick_event_mapper);


                romi::FakeWeeder weeder;
                
                // Navigation
                nlohmann::json rover_settings = config.at("navigation").at("rover");
                romi::NavigationSettings rover_config(rover_settings);
                std::string driver_device = config["ports"]["brush-motor-driver"]["port"];
                nlohmann::json driver_settings = config.at("navigation").at("brush-motor-driver");
                client_name = ("driver_device");
                auto driver_serial = romiserial::RomiSerialClient::create(driver_device,
                                                                          client_name,
                                                                          log);
                romi::BrushMotorDriver driver(driver_serial, driver_settings,
                                              rover_config.compute_max_angular_speed(),
                                              rover_config.compute_max_angular_acceleration());
                romi::WheelOdometry wheelodometry(rover_config, driver);
                romi::LocationTracker location_tracker(wheelodometry, wheelodometry);
                romi::ZeroNavigationController navigation_controller;

                romi::DifferentialSteering steering(driver, rover_config);

                romi::Navigation navigation(rover_config, location_tracker,
                                            location_tracker, navigation_controller,
                                            steering, session);

                // SpeedController
                romi::SpeedController speed_controller(navigation, config);

                // EventTimer
                romi::EventTimer event_timer(romi::event_timer_timeout);

                // Script engine
                romi::ScriptList scripts(get_script_file(options, config));
                romi::ScriptMenu menu(scripts);
                romi::FakeScriptEngine script_engine(scripts, romi::event_script_finished);

                // Notifications
                romi::FakeNotifications notifications;

                // Imager
                romi::FakeImager imager;

                romi::RemoteStateInputDevice remoteStateInputDevice;

                // Rover
                romi::Rover rover(input_device,
                                  display,
                                  speed_controller,
                                  navigation,
                                  event_timer,
                                  menu,
                                  script_engine,
                                  notifications,
                                  weeder,
                                  imager,
                                  remoteStateInputDevice);

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

                
        } catch (nlohmann::json::exception& je) {
                r_err("main: Failed to read the configuration file: %s", je.what());
                
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
