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
#include <rcom.h>

#include "UIOptions.h"
#include "UIFactory.h"
#include "DefaultSpeedController.h"
#include "ConfigurationFile.h"
#include "UserInterface.h"

using namespace romi;

int main(int argc, char** argv)
{
        int retval = 1;
        UIOptions options;
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        app_set_name("user-interface");

        r_debug("UserInterface: Using configuration file: '%s'",
                options.config_file);

        try {
                UIFactory ui_factory;
                JsonCpp config = JsonCpp::load(options.config_file);

                InputDevice& input_device = ui_factory.create_input_device(options,
                                                                           config);

                Display& display = ui_factory.create_display(options, config);
                
                Navigation& navigation = ui_factory.create_navigation(options,
                                                                      config);
                
                DefaultSpeedController speed_controller(navigation, config);

                UserInterface user_interface(input_device,
                                             display,
                                             speed_controller);


                while (!app_quit()) {
                        
                        try {
                                user_interface.handle_events();
                        
                        } catch (std::exception& e) {
                                
                                navigation.stop();
                                throw e;
                        }
                }

                retval = 0;

                
        } catch (std::exception& e) {
                r_err("main: std::exception: %s", e.what());
        }
        
        return retval;
}
