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

#include <Linux.h>
#include <Clock.h>
#include <ClockAccessor.h>
#include <rover/RoverOptions.h>
#include <log.h>
#include <RSerial.h>
#include <api/DataLogAccessor.h>
#include <iostream>
#include "VeDirectFrameHandler.h"


std::atomic<bool> quit(false);
static double last_print_time = 0;

std::map<std::string, std::string> RelevantDataNames {{"V", "battery-voltage"}, {"VS", "battery-aux-voltage"}, {"I","battery-current"} , {"P", "battery-instant-power"}, {"CE", "battery-consumed-a-h"}, {"SOC", "battery-state-of-charge"}, {"TTG", "battery-time-to-go"}, {"ALARM", "battery-alarm-active"}, {"AR", "battery-alarm-reason"}};

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


bool RelevantData(const char *veName)
{
    bool relevant = false;
    std::string name(veName);

    if (RelevantDataNames.find(name) != RelevantDataNames.end()) {
        relevant = true;
    }
    return relevant;
}

double ConvertDataVal(const char* veName, const char* veValue)
{
    std::string string_value(veValue);
    double value = 0;
    try {
        if(string_value == "OFF")
            value = 0;
        else if(string_value == "ON")
            value = 1;
        else
            value = std::strtod(string_value.c_str(), nullptr);
    }
    catch (std::exception& e)
    {
        r_err("Failed to convert field<%s> value<value> to double. ", veName, veValue);
        throw;
    }

    return value;
}


void PrintData(const VeDirectFrameHandler& frameHandler) {

    auto current_print_time = rpp::ClockAccessor::GetInstance()->time();
    if (current_print_time > last_print_time + 2)
    {
        std::cout << std::endl << std::endl << std::endl;
        std::string current_name;
        for ( int i = 0; i < frameHandler.veEnd; i++ ) {
            current_name = frameHandler.veName[i];
            if (RelevantData(current_name.c_str())) {
                romi::log_data(RelevantDataNames[current_name], ConvertDataVal(frameHandler.veName[i], frameHandler.veValue[i]));
                std::cout << RelevantDataNames[current_name] << ": \t" << ConvertDataVal(frameHandler.veName[i], frameHandler.veValue[i]) << std::endl;
            }
        }
        last_print_time = rpp::ClockAccessor::GetInstance()->time();
    }
}

int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();
        std::string port("/dev/ttyUSB0");

        r_log_init();
        r_log_set_app("romi-rover");

        std::string filename = "battery.txt";
        auto datalog = std::make_shared<romi::DataLog>(filename);
        romi::DataLogAccessor::set(datalog);
        
        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        try {

            rpp::Linux linux;
            romiserial::RSerial serial(port, 19200, 0);
            VeDirectFrameHandler veDirectFrameHandler;

            // Session
            r_info("main: Creating session");

            while (!quit) {

                try {
                    using namespace std::chrono_literals;
                    std::string output;

                    while ( serial.available() ) {
                        char next;
                        serial.read(next);
                        veDirectFrameHandler.rxData((uint8_t) next);
                    }

                    PrintData(veDirectFrameHandler);
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
