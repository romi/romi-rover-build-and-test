/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
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
#include <atomic>
#include <csignal>
#include <syslog.h>

#include <rcom/Linux.h>

#include <RomiSerialClient.h>
#include <RSerial.h>

#include <util/ClockAccessor.h>
#include <util/RomiSerialLog.h>
#include <util/Logger.h>
#include <hal/BrushMotorDriver.h>
#include <rover/Navigation.h>
#include <rover/NavigationSettings.h>
#include <rover/ZeroNavigationController.h>
#include <rover/LocationTracker.h>
#include <configuration/ConfigurationProvider.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <session/Session.h>
#include <rover/RoverOptions.h>
#include <rover/WheelOdometry.h>
#include <rpc/NavigationAdaptor.h>
#include <fake/FakeMotorDriver.h>
#include <rover/IMUTrackFollower.h>
#include <rover/L1NavigationController.h>
#include <rover/DifferentialSteering.h>

std::atomic<bool> quit(false);

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "navigation-distance segmentation fault");
                exit(signal);
        } else if (signal == SIGINT) {
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        } else {
                r_err("Unknown signam received %d", signal);
        }
}

static const char *kDistanceString = "distance";
static const char *kSpeedString = "speed";
static const char *kControllerString = "controller";
static const char *kDriverString = "driver";
static const char *kNavigationString = "navigation";

int main(int argc, char** argv)
{
        
        romi::RoverOptions options;
        romi::Option distance_option = { kDistanceString, true, "1.0",
                                         "The travel distance" };
        options.add_option(distance_option);
        
        romi::Option speed_option = { kSpeedString, true, "0.1", "The travel speed" };
        options.add_option(speed_option);
        
        romi::Option ctrl_option = { kControllerString, true,
                                     //kDriverString,
                                     kNavigationString,
                                     "The controller to use ('driver'=brush-motor-controller "
                                     "or 'navigation'=navigation controller)" };
        options.add_option(ctrl_option);
        
        options.parse(argc, argv);
        options.exit_if_help_requested();

        std::string distance_value = options.get_value(kDistanceString);
        if (distance_value.empty()) {
                r_err("No distance");
                return 1;
        }
        double distance = std::stod(distance_value);
        
        std::string speed_value = options.get_value(kSpeedString);
        double speed = std::stod(speed_value);

        if (distance < -50.0 || distance > 50.0) {
                r_err("Invalid distance: %f (must be in [-50, 50] range", distance);
                return 1;
        }

        if (speed < -1.0 || speed > 1.0) {
                r_err("Invalid speed: %f (must be in [-1, 1] range", speed);
                return 1;
        }
        
        r_info("Navigating %f meters at speed of %.1f%% of maximum speed",
               distance, 100.0 * speed);
        
        log_init();
        log_set_application("navigation");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        
        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());

                // TBD: USE FILEUTIL LOADER
                std::ifstream ifs(config_file);
                nlohmann::json config = nlohmann::json::parse(ifs);

                rcom::Linux linux;

                // Session
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationPrivider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = romi::get_session_directory(options, config);

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationPrivider));
                session.start("hw_observation_id");



                nlohmann::json rover_settings = config.at("navigation").at("rover");
                romi::NavigationSettings rover_config(rover_settings);
                
                nlohmann::json driver_settings = config.at("navigation").at("brush-motor-driver");

                std::shared_ptr<romiserial::ILog> log
                        = std::make_shared<romi::RomiSerialLog>();
                
                std::string device = romi::get_brush_motor_device(options, config);
                // std::shared_ptr<romiserial::RSerial> serial
                //         = std::make_shared<romiserial::RSerial>(device, 115200, 1);
                
                std::string client_name("navigation_distance_tests");
                auto romi_serial = romiserial::RomiSerialClient::create(device,
                                                                        client_name,
                                                                        log);
                        
                romi::BrushMotorDriver driver(romi_serial,
                                              driver_settings,
                                              rover_config.compute_max_angular_speed(),
                                              rover_config.compute_max_angular_acceleration());



                //romi::FakeMotorDriver driver;
                
                romi::WheelOdometry wheelodometry(rover_config, driver);
                
                romi::LocationTracker distance_measure(wheelodometry, wheelodometry);
                
                std::string imu_device = config["ports"]["imu"]["port"];
                client_name = "imu_device";
                auto imu_serial = romiserial::RomiSerialClient::create(imu_device,
                                                                       client_name,
                                                                       log);
                romi::IMUTrackFollower track_follower(imu_serial);
                
                //romi::ZeroNavigationController navigation_controller;
                romi::L1NavigationController navigation_controller(2.0);

                romi::DifferentialSteering steering(driver, rover_config);
                
                romi::Navigation navigation(rover_config, distance_measure,
                                            track_follower, navigation_controller,
                                            steering, session);


                std::string controller = options.get_value(kControllerString);
                
                //driver.start_recording_pid();
                // driver.start_recording_speeds();
                
                if (controller == kDriverString) {
                        r_info("Using motor driver");
                        driver.moveat(speed, speed);
                        romi::ClockAccessor::GetInstance()->sleep(5.0);
                        driver.moveat(0.0, 0.0);
                        romi::ClockAccessor::GetInstance()->sleep(5.0);
                        
                } else if (controller == kNavigationString) {
                        r_info("Using navigation controller");
                        navigation.move(distance, speed);
                }

                r_info("Recording PID and navigation speed data for 10 seconds...");
                romi::ClockAccessor::GetInstance()->sleep(10.0);
                
                //driver.stop_recording_pid();
                // driver.stop_recording_speeds();
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

