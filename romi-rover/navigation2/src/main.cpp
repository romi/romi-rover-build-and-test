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

#include <BrushMotorDriver.h>
#include <Navigation.h>
#include <NavigationSettings.h>
#include "configuration/ConfigurationProvider.h"
#include <rover/RoverOptions.h>
#include <rpc/NavigationAdaptor.h>

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

        // TBD: Check with Peter.
//        app_init(&argc, argv);
//        app_set_name("navigation");
        
        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());
                JsonCpp config = JsonCpp::load(config_file.c_str());

                JsonCpp rover_settings = config["navigation"]["rover"];
                romi::NavigationSettings rover(rover_settings);
                
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                
                std::string device = romi::get_brush_motor_device(options, config);
                std::shared_ptr<RSerial>serial = std::make_shared<RSerial>(device, 115200, 1);
                RomiSerialClient romi_serial(serial, serial);
                romi_serial.set_debug(true);

                romi::BrushMotorDriver driver(romi_serial,
                                        driver_settings,
                                        static_cast<int>(rover.encoder_steps),
                                        rover.max_revolutions_per_sec);

                romi::Navigation navigation(driver, rover);

                romi::NavigationAdaptor adaptor(navigation);
                auto server = romi::RcomServer::create("navigation", adaptor);

                while (!quit) {
                        server->handle_events();
                        clock_sleep(0.010);
                }
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

