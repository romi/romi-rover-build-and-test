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
#ifndef __ROMI_UI_OPTIONS_H
#define __ROMI_UI_OPTIONS_H

namespace romi {
        
        struct UIOptions {
        
                const char *config_file;
                const char *input_device_classname;
                const char *joystick_device;
                const char *navigation_classname;
                const char *navigation_server_name;
                const char *display_device;
                const char *display_classname;

                UIOptions() {
                        config_file = "config.json";
                        display_classname = 0;
                        display_device = 0;
                        input_device_classname = 0;
                        joystick_device = 0;
                        navigation_classname = 0;
                        navigation_server_name = 0;
                }

                void parse(int argc, char** argv);
                
                void print_usage();
                const char *get_option_message(int short_option);
        };
}

#endif // __ROMI_UI_OPTIONS_H
