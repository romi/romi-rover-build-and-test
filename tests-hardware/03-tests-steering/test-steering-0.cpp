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
#include <string>
#include <iostream>
#include <stdexcept>
#include <math.h>

#include <syslog.h>
#include <rpc/RcomServer.h>
#include <RomiSerialClient.h>
#include <RSerial.h>
#include <ClockAccessor.h>

#include <hal/BrushMotorDriver.h>
#include <rover/Navigation.h>
#include <rover/NavigationSettings.h>
#include <rover/ZeroNavigationController.h>
#include <rover/LocationTracker.h>
#include <rover/StepperSteering.h>
#include <configuration/ConfigurationProvider.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <rover/RoverOptions.h>
#include <rover/SteeringController.h>
#include <rpc/NavigationAdaptor.h>
#include <fake/FakeMotorDriver.h>
#include <oquam/StepperController.h>
#include <oquam/StepperSettings.h>

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
        
        romi::RoverOptions options;
        
        options.parse(argc, argv);
        options.exit_if_help_requested();
        
        r_log_init();
        r_log_set_app("navigation");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        
        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());
                JsonCpp config = JsonCpp::load(config_file.c_str());

                JsonCpp rover_settings = config["navigation"]["rover"];
                romi::NavigationSettings rover_config(rover_settings);
                
                const char *steering_device = (const char *) config["ports"]["steering"]["port"];
                auto steering_serial = romiserial::RomiSerialClient::create(steering_device);
                romi::SteeringController steering_controller(steering_serial);
                //romi::StepperController steering_controller(steering_serial);
                double max_rpm = 500; // From the motor specs
                double max_rps = max_rpm / 60.0;
                double default_rps = max_rps / 2.0; // Turn at 1/2th of max speed
                double steps_per_revolution = 200; // From the motor specs
                double microsteps = 2;  // Driver jumper settings
                double gears = 76.0 + 49.0/64.0; // From the motor specs
                double belt = 34.0 / 34.0;
                int16_t steps_per_second = (int16_t) ceil(default_rps
                                                          * steps_per_revolution
                                                          * microsteps);
                double total_steps_per_revolution = (steps_per_revolution
                                                     * microsteps * gears * belt);
                
                romi::StepperSteering wheel_steering(steering_controller, rover_config,
                                                     steps_per_second,
                                                     total_steps_per_revolution);

                wheel_steering.turn_wheels(0, 0);

                rpp::ClockAccessor::GetInstance()->sleep(2.0);

                r_info("Moving to 45 degrees");
                wheel_steering.turn_wheels(M_PI/4, M_PI/4);
                
                rpp::ClockAccessor::GetInstance()->sleep(15.0);

                r_info("Moving to 180 degrees");
                wheel_steering.turn_wheels(M_PI, M_PI);
                
                rpp::ClockAccessor::GetInstance()->sleep(15.0);

                r_info("Moving to -180 degrees");
                wheel_steering.turn_wheels(-M_PI, -M_PI);
                
                rpp::ClockAccessor::GetInstance()->sleep(20.0);

                r_info("Moving to -45 degrees");
                wheel_steering.turn_wheels(-M_PI/4, -M_PI/4);
                
                rpp::ClockAccessor::GetInstance()->sleep(15.0);

                r_info("Moving to 0 degrees");
                wheel_steering.turn_wheels(0, 0);
                
                rpp::ClockAccessor::GetInstance()->sleep(10.0);
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

