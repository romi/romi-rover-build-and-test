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
#include <r.h>
#include "RoverOptions.h"

using namespace std;

namespace romi {
        
        static Option _options[] = {
                { "help", false, 0,
                  "Print help message" },

                // All
                
                { "config", true, "config.json",
                  "Path of the config file" },
                
                { "session-directory", true, ".",
                  "The session directory where the output "
                  "files are stored (logs, images...)"},

                // User interface
                
                { "script-file", true, "script.json",
                  "The path of the file containing the scripts and menus" },
                
                { "input-device-classname", true, 0,
                  "The classname of the input device. "
                  "Options: joystick, fake-input-device"},
                
                { "joystick-device", true, 0,
                  "The path of the system device for the input device" },
                
                { "navigation-classname", true, 0,
                  "The classname of the navigation module. "
                  "Options: remote-navigation, fake-navigation"},
                
                { "display-classname", true, 0,
                  "The classname of the display module"
                  "Options: crystal-display, fake-display" },
                
                { "display-device", true, 0,
                  "The path of the system device for the display "},

                { "notifications-sound-font", true, 0,
                  "The path of the soundfont for the sound notification "},

                { "weeder-classname", true, 0,
                  "The classname of the weeder module used by the user interface "
                  "Options: weeder, fake-weeder "},

                // Weeder

                {"weeder-cnc-classname", true, 0, 
                  "The classname of the CNC module. "
                  "Options: remote-cnc, fake-cnc"},
                
                {"weeder-camera-classname", true, 0, 
                  "The classname of the camera. "
                  "Options: usb-camera-usb, file-camera"},
                
                {"weeder-camera-image", true, 0, 
                  "The path of the image file for the file camera."},
                
                {"weeder-camera-device", true, 0, 
                  "The device path for the USB camera."},
                
                // Oquam
                
                { "oquam-controller-classname", true, 0,
                  "The classname of oquam's low-level CNC controller. "
                  "Options: stepper-controller, fake-controller"
                },
                
                { "oquam-controller-device", true, 0,
                  "The stepper controller's serial device "},

                // Navigation
                
                { "navigation-driver-classname", true, 0,
                  "The classname of navigation's low-level driver. "
                  "Options: brush-motor-driver, fake-driver"
                },
                
                { "navigation-driver-device", true, 0,
                  "The brush motor driver's serial device"
                },
        };
        
        Option *rover_options = _options;

        size_t rover_options_length = (sizeof(_options) / sizeof(Option));

}

