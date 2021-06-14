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

#include <RomiSerialClient.h>
#include <RSerial.h>
#include <Linux.h>
#include <ClockAccessor.h>

#include <hal/BrushMotorDriver.h>
#include <rover/Navigation.h>
#include <rover/NavigationSettings.h>
#include <configuration/ConfigurationProvider.h>
#include <rover/RoverOptions.h>
#include <rover/WheelOdometry.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <session/Session.h>
#include <rpc/NavigationAdaptor.h>
#include <rpc/RcomServer.h>

std::atomic<bool> quit(false);

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "navigation segmentation fault");
                exit(signal);
        }
        else if (signal == SIGINT){
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        }
        else{
                r_err("Unknown signal received %d", signal);
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

                // Session
                r_info("main: Creating session");
                rpp::Linux linux;
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = romi::get_session_directory(options, config);

                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));
                session.start("hw_observation_id");


                JsonCpp rover_settings = config["navigation"]["rover"];
                romi::NavigationSettings navigation_settings(rover_settings);
                
                JsonCpp driver_settings = config["navigation"]["brush-motor-driver"];
                
                std::string device = romi::get_brush_motor_device(options, config);
                std::shared_ptr<romiserial::RSerial>serial = std::make_shared<romiserial::RSerial>(device, 115200, 1);
                auto romi_serial = romiserial::RomiSerialClient::create(device);
                //romi_serial.set_debug(true);

                romi::BrushMotorDriver driver(romi_serial,
                                              driver_settings,
                                              (int) navigation_settings.encoder_steps,
                                              navigation_settings.max_revolutions_per_sec);

                romi::WheelOdometry wheelodometry(navigation_settings, driver);
                romi::Navigation navigation(driver, navigation_settings,
                                            wheelodometry, session);

                romi::NavigationAdaptor adaptor(navigation);
                auto server = romi::RcomServer::create("navigation", adaptor);

                while (!quit) {
                        server->handle_events();
                        rpp::ClockAccessor::GetInstance()->sleep(0.010);
                }
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

