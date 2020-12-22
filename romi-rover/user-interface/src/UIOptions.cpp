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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "UIOptions.h"

namespace romi {
        
        static const char *optchars = "hC:N:T:D:n:T:";
        
        static struct option long_options[] = {
                {"help", no_argument, 0, 'h'},
                {"config", required_argument, 0, 'C'},
                {"input-device-classname", required_argument, 0, 'I'},
                {"joystick-device", required_argument, 0, 'J'},
                {"navigation-classname", required_argument, 0, 'n'},
                {"navigation-server-name", required_argument, 0, 'N'},
                {"display-device", required_argument, 0, 'D'},
                {"display-classname", required_argument, 0, 'T'},
                {0, 0, 0, 0}
        };
        
        void UIOptions::parse(int argc, char** argv)
        {
                int option_index;
                
                while (1) {
                        int c = getopt_long(argc, argv, optchars,
                                            long_options, &option_index);
                        if (c == -1) break;
                        switch (c) {
                        case 'h':
                                print_usage();
                                exit(0);
                                break;
                        case 'C':
                                config_file = optarg;
                                break;
                        case 'n':
                                navigation_classname = optarg;
                                break;
                        case 'N':
                                navigation_server_name = optarg;
                                break;
                        case 'I':
                                input_device_classname = optarg;
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

        const char *UIOptions::get_option_message(int short_option)
        {
                const char *message = "<TODO: supply help message>";
                switch (short_option) {
                case 'h':
                        message = "Print usage information.";
                        break;
                case 'C':
                        message = "Load the given configuration file.";
                        break;
                case 'n':
                        message = "The classname of the navigation module.";
                        break;
                case 'N':
                        message = "The server name of the remote navigation module.";
                        break;
                case 'I':
                        message = "The classname of the input device.";
                        break;
                case 'J':
                        message = "The joystick device path.";
                        break;
                case 'T':
                        message = "The classname of the display module.";
                        break;
                case 'D':
                        message = "The device used by the display module.";
                        break;
                }
                return message;
        }
        
        void UIOptions::print_usage()
        {
                printf("Usage: user-interface <options>\n");
                printf("Available options:\n");
                for (int i = 0; long_options[i].name != 0; i++) {
                        printf("--%s, -%c",
                               long_options[i].name,
                               (char)long_options[i].val);
                        if (long_options[i].has_arg == required_argument)
                                printf(" argument");
                        else if (long_options[i].has_arg == optional_argument)
                                printf(" [argument]");
                        printf(": %s\n", get_option_message(long_options[i].val));
                }
        }
               
}
