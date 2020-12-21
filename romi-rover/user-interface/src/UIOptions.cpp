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
#include <getopt.h>

#include "UIOptions.h"

namespace romi {
        
        void UIOptions::parse(int argc, char** argv)
        {
                int option_index;
                static const char *optchars = "C:N:T:D:n:T:";
                static struct option long_options[] = {
                        {"config", required_argument, 0, 'C'},
                        {"joystick-device", required_argument, 0, 'J'},
                        {"navigation-classname", required_argument, 0, 'n'},
                        {"navigation-server-name", required_argument, 0, 'N'},
                        {"display-device", required_argument, 0, 'D'},
                        {"display-classname", required_argument, 0, 'T'},
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
                                navigation_classname = optarg;
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
                                display_classname = optarg;
                                break;
                        }
                }
        }
}
