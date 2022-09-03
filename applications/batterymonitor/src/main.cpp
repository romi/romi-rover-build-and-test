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
#include <thread>
#include <map>
#include <algorithm>

#include <rcom/Linux.h>
#include <util/Clock.h>
#include <util/ClockAccessor.h>
#include <rover/RoverOptions.h>
#include <util/Logger.h>
#include <util/RomiSerialLog.h>
#include <RSerial.h>
#include <api/DataLogAccessor.h>
#include <iostream>
#include <battery_monitor/BatteryMonitor.h>


std::atomic<bool> quit(false);

//std::map<std::string, std::string> RelevantDataNames {{"V", "battery-voltage"}, {"VS", "battery-aux-voltage"}, {"I","battery-current"} , {"P", "battery-instant-power"}, {"CE", "battery-consumed-a-h"}, {"SOC", "battery-state-of-charge"}, {"TTG", "battery-time-to-go"}, {"ALARM", "battery-alarm-active"}, {"AR", "battery-alarm-reason"}};

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
        std::string port("/dev/ttyUSB0");

        log_init();
        log_set_application("romi-rover");

        std::string filename = "battery.txt";
        auto datalog = std::make_shared<romi::DataLog>(filename);
        romi::DataLogAccessor::set(datalog);
        
        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        try {

            rcom::Linux linux;
            // Battery Monitor

            std::unique_ptr<romi::BatteryMonitor> battery_mon(nullptr);

            try {
                // Battery Monitor
                r_info("main: Creating Battery Monitor");
                const char *battery_monotor_device = "/dev/ttyACM0";
                auto client_name = "battery-monitor";
                std::shared_ptr<romiserial::ILog> log
                        = std::make_shared<romi::RomiSerialLog>();
               auto battery_serial
                        = romiserial::RomiSerialClient::create(battery_monotor_device,
                                                               client_name, log);

                battery_mon
                        = std::make_unique<romi::BatteryMonitor>(battery_serial,
                                                                 datalog, quit);
                battery_mon->enable();
            } catch (std::exception& e) {
                r_err("main: Unable to create battery monitor: %s", e.what());
            }

            // Session
            r_info("main: Creating session");

            while (!quit) {

                try {
                    using namespace std::chrono_literals;
                    std::string output;

                    std::this_thread::sleep_for(5ms);
                } catch (std::exception &e) {
                    quit = true;
                    throw;
                }
            }
            
            retval = 0;
            
        } catch (std::exception& e) {
                r_err("main: exception: %s", e.what());
        }
        
        return retval;
}
