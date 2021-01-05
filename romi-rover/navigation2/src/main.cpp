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

#include <rcom.h>
#include <RPCServer.h>

#include "BrushMotorDriver.h"
#include "RomiSerialClient.h"
#include "RSerial.h"
#include "RPCNavigationServerAdaptor.h"
#include "RoverNavigation.h"
#include "RoverConfiguration.h"
#include "RoverOptions.h"

using namespace romi;
using namespace rcom;
        
int main(int argc, char** argv)
{
        GetOpt options(rover_options, rover_options_length);
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        
        try {
                r_debug("Weeder: Using configuration file: '%s'",
                        options.get_value("config-file"));
                
                JsonCpp config = JsonCpp::load(options.get_value("config-file"));

                JsonCpp rover_settings = config["navigation"]["rover"];
                RoverConfiguration rover(rover_settings);
                
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                
                // TODO: check for serial_device in config
                RSerial serial(options.get_value("navigation-driver-device"), 115200, 1);
                RomiSerialClient romi_serial(&serial, &serial);
                romi_serial.set_debug(true);
                
                BrushMotorDriver driver(romi_serial,
                                        driver_settings,
                                        rover.encoder_steps,
                                        rover.max_revolutions_per_sec);

                RoverNavigation navigation(driver, rover);
                
                RPCNavigationServerAdaptor adaptor(navigation);
                RPCServer server(adaptor, "navigation", "navigation");
                
                while (!app_quit())
                        clock_sleep(0.1);

                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

