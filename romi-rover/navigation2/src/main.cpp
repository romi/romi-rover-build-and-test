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
#include <string.h>
#include <getopt.h>

#include <rcom.h>
#include <RPCServer.h>

#include "BrushMotorDriver.h"
#include "ConfigurationFile.h"
#include "RomiSerialClient.h"
#include "RSerial.h"
#include "ConfigurationFile.h"
#include "RPCNavigation.h"
#include "Navigation.h"
#include "RoverConfiguration.h"

using namespace romi;
using namespace rcom;

struct Options {
        
        const char *config_file;
        const char *serial_device;
        const char *server_name;
        const char *server_topic;

        Options() {
                config_file = "config.json";
                serial_device = "/dev/ttyACM0";
                server_name = "navigation";
                server_topic = "navigation";
        }

        void parse(int argc, char** argv) {
                int option_index;
                static const char *optchars = "C:N:T:D:";
                static struct option long_options[] = {
                        {"config", required_argument, 0, 'C'},
                        {"device", required_argument, 0, 'D'},
                        {"server-name", required_argument, 0, 'N'},
                        {"server-topic", required_argument, 0, 'T'},
                        {0, 0, 0, 0}
                };
        
                while (1) {
                        int c = getopt_long(argc, argv, optchars,
                                            long_options, &option_index);
                        if (c == -1) break;
                        switch (c) {
                        case 'C':
                                config_file = optarg;
                                break;
                        case 'N':
                                server_name = optarg;
                                break;
                        case 'T':
                                server_topic = optarg;
                                break;
                        case 'D':
                                serial_device = optarg;
                                break;
                        }
                }
        }
};
        
int main(int argc, char** argv)
{
        Options options;
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        
        try {
                r_debug("Weeder: Using configuration file: '%s'", options.config_file);
                ConfigurationFile config(options.config_file);

                JSON navigation_settings = config.get("navigation");

                JSON rover_settings = navigation_settings.get("rover");
                RoverConfiguration rover(rover_settings);
                
                JSON driver_settings = navigation_settings.get("brush-motor-driver");

                
                
                // TODO: check for serial_device in config
                RSerial serial(options.serial_device, 115200, 1);        
                RomiSerialClient romi_serial(&serial, &serial);
                BrushMotorDriver driver(romi_serial,
                                        driver_settings,
                                        rover.encoder_steps,
                                        rover.max_revolutions_per_sec);

                Navigation navigation(driver, rover);
                
                RPCNavigation navigation_adaptor(navigation);
                RPCServer server(navigation_adaptor,
                                 options.server_name,
                                 options.server_topic);
                
                while (!app_quit())
                        clock_sleep(0.1);

                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

