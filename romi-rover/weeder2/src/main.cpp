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

#include <RPCServer.h>

#include "FakeCNC.h"
#include "ConfigurationFile.h"
#include "CameraFactory.h"
#include "CameraServer.h"
#include "Weeder.h"
#include "Pipeline.h"
#include "DebugWeedingSession.h"

using namespace romi;

struct Options {
        
        const char *config_file;
        const char *server_name;
        const char *camera_topic;
        const char *weeder_topic;
        const char *output_directory;

        Options() {
                config_file = "config.json";
                server_name = "weeder";
                camera_topic = "topcam";
                weeder_topic = "weeder";
                output_directory = ".";
        }

        void parse(int argc, char** argv) {
                int option_index;
                static const char *optchars = "C:N:W:T:d:";
                static struct option long_options[] = {
                        {"config", required_argument, 0, 'C'},
                        {"server-name", required_argument, 0, 'N'},
                        {"camera-topic", required_argument, 0, 'T'},
                        {"weeder-topic", required_argument, 0, 'W'},
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
                        case 'N':
                                server_name = optarg;
                                break;
                        case 'T':
                                camera_topic = optarg;
                                break;
                        case 'W':
                                weeder_topic = optarg;
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
                r_debug("Weeder: Using configuration file: '%s'", options.config_file);
                ConfigurationFile config(options.config_file);

                // Instantiate the camera
                const char *camera_class = config.get("weeder").str("camera");
                JSON camera_config = config.get("weeder").get(camera_class);
                ICamera *camera = CameraFactory::create(camera_class, camera_config);
                
                // Make the camera accessible over HTTP 
                CameraServer camera_server(*camera, options.server_name,
                                           options.camera_topic);
                
                ICNC *cnc = 0;
#if 0
                IController *controller = new ControllerClient("weeder", "cnc");
                cnc = new CNCProxy(controller);
#else
                cnc = new FakeCNC(config);
#endif
                
                CNCRange range;
                cnc->get_range(range);
                
                r_debug("main: range: %f, %f", range._x[0], range._x[1]);
                               
                IPipeline *pipeline = new Pipeline(range, config.get());
                
                DebugWeedingSession session(options.output_directory, "weeder");
                
                Weeder weeder(&config, camera, pipeline, cnc, range, session);
                
                rcom::RPCServer weeder_server(weeder,
                                              options.server_name,
                                              options.weeder_topic);
                
                while (!app_quit())
                        clock_sleep(0.1);

                delete cnc;
                delete pipeline;
#if 0
                delete controller;
#endif
                delete camera;
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

