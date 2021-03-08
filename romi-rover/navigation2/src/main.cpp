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

#include <rcom.h>
#include <RPCServer.h>
#include <RomiSerialClient.h>
#include <RSerial.h>

#include <BrushMotorDriver.h>
#include <Navigation.h>
#include <NavigationSettings.h>
#include <rover/RoverOptions.h>
#include <rpc/NavigationAdaptor.h>

using namespace romi;
using namespace rcom;

const char *get_brush_motor_device_in_config(JsonCpp& config)
{
        const char *brush_motor_device = nullptr;
        try {
                brush_motor_device = (const char *)config["ports"]["brush-motor-driver"]["port"];
                        
        } catch (JSONError &je) {
                r_warn("Failed to get the value for "
                       "ports.brush_motor.port: %s", je.what());
                throw std::runtime_error("No brush_motor device specified");
        }
        return brush_motor_device;
}

const char *get_brush_motor_device(Options& options, JsonCpp& config)
{
        const char *brush_motor_device = options.get_value(RoverOptions::navigation_device);
        if (brush_motor_device == nullptr) {
                brush_motor_device = get_brush_motor_device_in_config(config);
        }
        return brush_motor_device;
}

const char *get_config_file(Options& options)
{
        const char *file = options.get_value(RoverOptions::config);
        if (file == nullptr) {
                throw std::runtime_error("No configuration file was given (can't run without one...).");
        }
        return file;
}
        
int main(int argc, char** argv)
{
        RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();
        
        app_init(&argc, argv);
        app_set_name("navigation");
        
        try {
                const char *config_file = get_config_file(options);
                r_info("Navigation: Using configuration file: '%s'", config_file);
                JsonCpp config = JsonCpp::load(config_file);

                JsonCpp rover_settings = config["navigation"]["rover"];
                NavigationSettings rover(rover_settings);
                
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                
                const char *device = get_brush_motor_device(options, config);
                std::shared_ptr<RSerial>serial = std::make_shared<RSerial>(device, 115200, 1);
                RomiSerialClient romi_serial(serial, serial);
                romi_serial.set_debug(true);
                
                BrushMotorDriver driver(romi_serial,
                                        driver_settings,
                                        static_cast<int>(rover.encoder_steps),
                                        rover.max_revolutions_per_sec);

                Navigation navigation(driver, rover);
                
                NavigationAdaptor adaptor(navigation);
                RPCServer server(adaptor, "navigation", "navigation");
                
                while (!app_quit())
                        clock_sleep(0.1);

                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

