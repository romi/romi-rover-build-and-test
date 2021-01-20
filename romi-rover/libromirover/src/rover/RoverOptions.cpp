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
#include <stdexcept>
#include <r.h>
#include "rover/RoverOptions.h"

using namespace std;

namespace romi {
        
        static Option _options[] = {
                { "help", false, 0,
                  "Print help message" },

                // File paths
                
                { RoverOptions::config, true, "config.json",
                  "Path of the config file" },

                { RoverOptions::script, true, "script.json",
                  "The path of the file containing the scripts and menus" },
                
                { RoverOptions::session_directory, true, ".",
                  "The session directory where the output "
                  "files are stored (logs, images...)"},
                
                { RoverOptions::camera_image, true, 0, 
                  "The path of the image file for the file camera."},
                
                { RoverOptions::soundfont, true, 0,
                  "The path of the soundfont for the sound notification "},

                { RoverOptions::joystick_device, true, 0,
                  "The path of the system device for the input device" },
                
                { RoverOptions::display_device, true, 0,
                  "The path of the system device for the display "},
                
                { RoverOptions::camera_device, true, 0, 
                  "The device path for the USB camera."},
                
                { RoverOptions::cnc_device, true, 0,
                  "The stepper controller's serial device "},
                
                { RoverOptions::navigation_device, true, 0,
                  "The brush motor driver's serial device"
                },
        };
        
        static Option *rover_options = _options;
        static size_t rover_options_length = (sizeof(_options) / sizeof(Option));


        RoverOptions::RoverOptions()
                : GetOpt(rover_options, rover_options_length)
        {}
        
        void RoverOptions::exit_if_help_requested()
        {
                if (is_help_requested()) {
                        print_usage();
                        exit(0);
                }
        }

        // const char *RoverOptions::get_config_file()
        // {
        //         const char *file = get_value(RoverOptions::config);
        //         if (file == 0) {
        //                 throw std::runtime_error("No configuration file was given "
        //                                          "(can't run without one...).");
        //         }
        //         return file;
        // }

}

