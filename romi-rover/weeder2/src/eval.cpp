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
#include <exception>
#include <string.h>
#include <getopt.h>
#include <rcom.h>

#include "FakeCNC.h"
#include "CameraFile.h"
#include "ConfigurationFile.h"

#include "Weeder.h"
#include "Pipeline.h"
#include "DebugWeedingSession.h"

using namespace romi;

struct Options {
        
        const char *config_file;
        const char *input_image;
        const char *output_name;
        const char *output_directory;

        Options() {
                config_file = "config.json";
                input_image = "camera.jpg";
                output_name = "weeder";
                output_directory = ".";
        }

        void parse(int argc, char** argv) {
                int option_index;
                static const char *optchars = "C:I:O:d:";
                static struct option long_options[] = {
                        {"config", required_argument, 0, 'C'},
                        {"image", required_argument, 0, 'I'},
                        {"output-name", required_argument, 0, 'O'},
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
                        case 'I':
                                input_image = optarg;
                                break;
                        case 'O':
                                output_name = optarg;
                                break;
                        case 'd':
                                output_directory = optarg;
                                break;
                        }
                }
        }
};

int main(int argc, char** argv)
{
        Options options;
        options.parse(argc, argv);
        
        app_init(&argc, argv);

        try {
                ConfigurationFile config(options.config_file);
                CameraFile camera(options.input_image);
                FakeCNC cnc(config);
                CNCRange range;
                cnc.get_range(range);
                Pipeline pipeline(range, config.get());
                DebugWeedingSession session(options.output_directory, options.output_name);
                Weeder weeder(&config, &camera, &pipeline, &cnc, range, session);
                
                weeder.hoe();
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

