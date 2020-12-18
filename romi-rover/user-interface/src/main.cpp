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
#include <string.h>
#include <getopt.h>
#include <rcom.h>

#include "Joystick.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "FakeNavigation.h"
#include "RPCNavigationClientAdaptor.h"
#include "RPCClient.h"
#include "SpeedController.h"
#include "UIStateTransitions.h"
#include "EventMapper.h"
#include "ConfigurationFile.h"
#include "FakeDisplay.h"
#include "CrystalDisplay.h"
#include "RomiSerialClient.h"
#include "RSerial.h"
#include "UserInterface.h"

using namespace romi;

static RSerial *serial = 0;
static RomiSerialClient *romi_serial = 0;
static IDisplay *display = 0;
static rcom::RPCClient *rpc_client = 0; 
static INavigation *navigation = 0;

struct Options {
        
        const char *config_file;
        const char *joystick_device;
        const char *navigation_class;
        const char *navigation_server_name;
        const char *display_device;
        const char *display_type;

        Options() {
                config_file = "config.json";
                display_type = 0;
                display_device = "/dev/ttyACM0";
                joystick_device = "/dev/input/js0";
                navigation_class = 0;
                navigation_server_name = "navigation";
        }

        void parse(int argc, char** argv) {
                int option_index;
                static const char *optchars = "C:N:T:D:n:T:";
                static struct option long_options[] = {
                        {"config", required_argument, 0, 'C'},
                        {"joystick-device", required_argument, 0, 'J'},
                        {"navigation-class", required_argument, 0, 'n'},
                        {"navigation-server-name", required_argument, 0, 'N'},
                        {"display-device", required_argument, 0, 'D'},
                        {"display-type", required_argument, 0, 'T'},
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
                        case 'n':
                                navigation_class = optarg;
                                break;
                        case 'N':
                                navigation_server_name = optarg;
                                break;
                        case 'J':
                                joystick_device = optarg;
                                break;
                        case 'D':
                                display_device = optarg;
                                break;
                        case 'T':
                                display_type = optarg;
                                break;
                        }
                }
        }
};

const char *get_display_type(Options &options, IConfiguration &config)
{
        const char *display_type = options.display_type;
        if (display_type == 0) {
                try {
                        display_type = config.get("user-interface").str("display");
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for user-interface.display: %s", je.what());
                }
        }
        return display_type;
}

IDisplay *create_display(Options &options, IConfiguration &config)
{
        const char *display_type = get_display_type(options, config);
        if (display_type == 0)
                throw std::runtime_error("No display type was defined in the options "
                                         "or in the configuration file.");

        IDisplay *display = 0;
        
        if (rstreq(display_type, FakeDisplay::ClassName)) {
                display = new FakeDisplay();

        } else if (rstreq(display_type, CrystalDisplay::ClassName)) {
                serial = new RSerial(options.display_device, 115200, 1);
                romi_serial = new RomiSerialClient(serial, serial);

                romi_serial->set_debug(true);
                
                display = new CrystalDisplay(*romi_serial);
        }

        if (display == 0) {
                r_err("Failed to create the '%s' display", display_type);
                throw std::runtime_error("Failed to create the display");
        }
        
        return display;
}

const char *get_navigation_class(Options &options, IConfiguration &config)
{
        const char *navigation_class = options.navigation_class;
        if (navigation_class == 0) {
                try {
                        navigation_class = config.get("user-interface").str("navigation");
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for user-interface.navigation: %s", je.what());
                }
        }
        return navigation_class;
}

INavigation *create_navigation(Options &options, IConfiguration &config)
{
        const char *navigation_class = get_navigation_class(options, config);
        if (navigation_class == 0)
                throw std::runtime_error("No navigation type was defined in the options "
                                         "or in the configuration file.");

        INavigation *navigation = 0;
        
        if (rstreq(navigation_class, FakeNavigation::ClassName)) {
                navigation = new FakeNavigation();

        } else if (rstreq(navigation_class, RPCNavigationClientAdaptor::ClassName)) {
                rpc_client = new rcom::RPCClient(options.navigation_server_name, "navigation");
                navigation = new RPCNavigationClientAdaptor(*rpc_client);
        }

        if (navigation == 0) {
                r_err("Failed to create an instance of '%s' ", navigation_class);
                throw std::runtime_error("Failed to create the navigation module");
        }
        
        return navigation;
}

int main(int argc, char** argv)
{
        int retval = 1;
        Options options;
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        app_set_name("user-interface");

        r_debug("UserInterface: Using configuration file: '%s'",
                options.config_file);

        
        try {
                ConfigurationFile config(options.config_file);
 
                JoystickEvent event;
                Joystick joystick(options.joystick_device);

                display = create_display(options, config);
                
                navigation = create_navigation(options, config);
                        
                JsonCpp ui_config = config.get("user-interface");
                SpeedController speed_controller(*navigation, ui_config);

                UserInterface user_interface(joystick, *display, speed_controller);
                
                StateMachine state_machine(user_interface);

                init_state_transitions(state_machine);
                state_machine.handleEvent(event_start, 0);

                
                EventMapper eventMapper(joystick);

                while (!app_quit()) {
                        
                        if (joystick.update(event)) {
                                int16_t state_event = eventMapper.map(event);
                                if (state_event > 0)
                                        state_machine.handleEvent(state_event, 0);
                        } else {
                                clock_sleep(0.010);
                        }
                        
                        
                        // Status status;
                        
                        // navigation.get_status(status);
                        
                        // if (status.code == Status::Error) {
                        //         state_machine.handle_event(event_error);
                        // } else {
                        //         weeder.get_status(status);
                        //         if (status.code == Status::Error) {
                        //                 state_machine.handle_event(event_error);
                        //         } else {
                        //                 state_machine.handle_event(event_ready);
                        //         }
                        // }

                }

                retval = 0;

                
        } catch (std::exception& e) {
                r_err("main: std::exception: %s", e.what());
        }

        
        if (navigation)
                delete navigation;
        
        if (rpc_client)
                delete rpc_client;
        
        if (display)
                delete display;
        
        if (romi_serial)
                delete romi_serial;
        
        if (serial)
                delete serial;
        
        return retval;
}
