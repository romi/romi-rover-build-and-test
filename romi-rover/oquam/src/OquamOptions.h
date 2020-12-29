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
#ifndef __ROMI_OQUAM_OPTIONS_H
#define __ROMI_OQUAM_OPTIONS_H

#include <getopt.h>

namespace romi {

        struct OquamOptions {
        
                const char *config_file;
                const char *serial_device;
                const char *server_name;
                const char *cnc_controller;
                const char *output_directory;

                OquamOptions() {
                        config_file = "config.json";
                        serial_device = "/dev/ttyACM0";
                        server_name = "oquam";
                        cnc_controller = "oquam";
                        output_directory = ".";
                }

                void parse(int argc, char** argv);
        };
}

#endif // __ROMI_OQUAM_OPTIONS_H
