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

#include <stdexcept>
#include "UIFactory.h"
#include "FakeNavigation.h"
#include "RemoteNavigation.h"
#include "FakeDisplay.h"
#include "CrystalDisplay.h"

namespace romi {
        
        const char *UIFactory::get_display_classname(UIOptions &options, JsonCpp &config)
        {
                const char *display_classname = options.display_classname;
                if (display_classname == 0) {
                        try {
                                display_classname = config.str("display-classname");
                        } catch (JSONError &je) {
                                r_warn("Failed to get the value for "
                                       "user-interface.display: %s", je.what());
                        }
                }
                return display_classname;
        }

        void UIFactory::do_create_display(UIOptions &options, JsonCpp &config)
        {
                const char *display_classname = get_display_classname(options, config);
                if (display_classname == 0)
                        throw std::runtime_error("No display type defined in the options "
                                                 "or in the configuration file.");

                r_info("Create an instance of display '%s'", display_classname);
                
                if (rstreq(display_classname, FakeDisplay::ClassName)) {
                        _display = new FakeDisplay();

                } else if (rstreq(display_classname, CrystalDisplay::ClassName)) {
                        _serial = new RSerial(options.display_device, 115200, 1);
                        _romi_serial = new RomiSerialClient(_serial, _serial);

                        _romi_serial->set_debug(true);
                
                        _display = new CrystalDisplay(*_romi_serial);
                }
        }

        Display& UIFactory::create_display(UIOptions &options, JsonCpp &config)
        {
                if (_display == 0) {
                        do_create_display(options, config);
                }

                if (_display == 0) {
                        r_err("Failed to create the display");
                        throw std::runtime_error("Failed to create the display");
                }
        
                return *_display;
        }

        const char *UIFactory::get_navigation_classname(UIOptions &options,
                                                    JsonCpp &config)
        {
                const char *navigation_classname = options.navigation_classname;
                if (navigation_classname == 0) {
                        try {
                                navigation_classname = config.str("navigation-classname");
                        } catch (JSONError &je) {
                                r_warn("Failed to get the value for "
                                       "user-interface.navigation: %s", je.what());
                        }
                }
                return navigation_classname;
        }

        void UIFactory::do_create_navigation(UIOptions &options, JsonCpp &config)
        {
                const char *navigation_classname = get_navigation_classname(options, config);
                if (navigation_classname == 0)
                        throw std::runtime_error("No navigation type defined in the "
                                                 "options or in the configuration file.");
        
                r_info("Create an instance of navigation '%s'", navigation_classname);
                        
                if (rstreq(navigation_classname, FakeNavigation::ClassName)) {
                        _navigation = new FakeNavigation();

                } else if (rstreq(navigation_classname, RemoteNavigation::ClassName)) {
                        _rpc_client = new rcom::RPCClient(options.navigation_server_name,
                                                         "navigation");
                        _navigation = new RemoteNavigation(*_rpc_client);
                }
        }


        Navigation& UIFactory::create_navigation(UIOptions &options, JsonCpp &config)
        {
                if (_navigation == 0) {
                        do_create_navigation(options, config);
                }
                
                if (_navigation == 0) {
                        r_err("Failed to create an instance of navigation");
                        throw std::runtime_error("Failed to create the navigation module");
                }
        
                return *_navigation;
        }
}
