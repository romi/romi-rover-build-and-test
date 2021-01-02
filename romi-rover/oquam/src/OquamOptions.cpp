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
#include "OquamOptions.h"

namespace romi {

        void OquamOptions::parse(int argc, char** argv)
        {
                int option_index;
                static const char *optchars = "C:D:N:c:d:";
                static struct option long_options[] = {
                        {"config", required_argument, 0, 'C'},
                        {"controller-device", required_argument, 0, 'D'},
                        {"server-name", required_argument, 0, 'N'},
                        {"controller-classname", required_argument, 0, 'c'},
                        {"output-directory", required_argument, 0, 'd'},
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
                        case 'D':
                                controller_device = optarg;
                                break;
                        case 'N':
                                server_name = optarg;
                                break;
                        case 'c':
                                controller_classname = optarg;
                                break;
                        case 'd':
                                output_directory = optarg;
                                break;
                        }
                }
        }
}
