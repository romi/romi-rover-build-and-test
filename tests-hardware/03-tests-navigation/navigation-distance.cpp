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
#include <rpc/RcomServer.h>
#include <RomiSerialClient.h>
#include <RSerial.h>

#include <hal/BrushMotorDriver.h>
#include <rover/Navigation.h>
#include <rover/NavigationSettings.h>
#include "configuration/ConfigurationProvider.h"
#include <rover/RoverOptions.h>
#include <rpc/NavigationAdaptor.h>
#include <thread>

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
        romi::Option distance_option = { "distance", true, "1.0", "The travel distance" };
        options.add_option(distance_option);
        romi::Option speed_option = { "speed", true, "0.1", "The travel speed" };
        options.add_option(speed_option);
        options.parse(argc, argv);
        options.exit_if_help_requested();

        std::string distance_value = options.get_value("distance");
        if (distance_value.empty()) {
                r_err("No distance");
                return 1;
        }
        double distance = std::stod(distance_value);
        std::string speed_value = options.get_value("speed");
        double speed = std::stod(speed_value);

        if (distance < -10.0 || distance > 10.0) {
                r_err("Invalid distance: %f (must be in [-10, 10] range", distance);
                return 1;
        }

        if (speed < -1.0 || speed > 1.0) {
                r_err("Invalid speed: %f (must be in [-1, 1] range", speed);
                return 1;
        }
        
        r_info("Navigating %f meters at speed of %.0f%% of maximum speed",
               distance, 100.0 * speed);
        
        r_log_init();
        r_log_set_app("navigation");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        
        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());
                JsonCpp config = JsonCpp::load(config_file.c_str());

                JsonCpp rover_settings = config["navigation"]["rover"];
                romi::NavigationSettings rover(rover_settings);
                
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                
                std::string device = romi::get_brush_motor_device(options, config);
                std::shared_ptr<romiserial::RSerial>serial
                        = std::make_shared<romiserial::RSerial>(device, 115200, 1);
                auto romi_serial = romiserial::RomiSerialClient::create(device);
                //romi_serial.set_debug(true);

                romi::BrushMotorDriver driver(romi_serial,
                                              driver_settings,
                                              static_cast<int>(rover.encoder_steps),
                                              rover.max_revolutions_per_sec);

                romi::Navigation navigation(driver, rover);

                navigation.move(distance, speed);
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

