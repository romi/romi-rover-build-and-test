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
#include "RoverWeeder.h"
#include "Pipeline.h"
#include "DebugWeedingSession.h"
#include "RPCClient.h"
#include "RemoteCNC.h"

using namespace romi;
using namespace rcom;

struct Options {
        
        const char *config_file;
        const char *server_name;
        const char *camera_topic;
        const char *weeder_topic;
        const char *output_directory;
        const char *cnc_class;
        const char *cnc_name;
        const char *camera_class;

        Options() {
                config_file = "config.json";
                server_name = "weeder";
                camera_topic = "topcam";
                weeder_topic = "weeder";
                output_directory = ".";
                cnc_class = 0;
                cnc_name = "oquam";
                camera_class = 0;
        }

        void parse(int argc, char** argv) {
                int option_index;
                static const char *optchars = "C:N:W:T:d:c:n:a:";
                static struct option long_options[] = {
                        {"config", required_argument, 0, 'C'},
                        {"server-name", required_argument, 0, 'N'},
                        {"camera-topic", required_argument, 0, 'T'},
                        {"weeder-topic", required_argument, 0, 'W'},
                        {"output-directory", required_argument, 0, 'd'},
                        {"cnc-class", required_argument, 0, 'c'},
                        {"cnc-name", required_argument, 0, 'n'},
                        {"camera", required_argument, 0, 'a'},
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
                        case 'c':
                                cnc_class = optarg;
                                break;
                        case 'n':
                                cnc_name = optarg;
                                break;
                        case 'a':
                                camera_class = optarg;
                                break;
                        }
                }
        }
};

static CNC *cnc = 0;
static RPCClient *rpc_client = 0;
static Camera *camera = 0;

const char *get_camera_class(Options &options, IConfiguration &config)
{
        const char *camera_class = options.camera_class;
        if (camera_class == 0) {
                try {
                        camera_class = config.get("weeder").str("camera");
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for weeder.camera: %s", je.what());
                }
        }
        return camera_class;
}

void get_camera_config(const char *camera_class, IConfiguration &config, JsonCpp &camera_config)
{
        if (config.get().has("weeder")
            && config.get("weeder").has(camera_class))
                camera_config = config.get("weeder").get(camera_class);
        else 
                r_warn("Configuration does not have a '%s' camera section",
                       camera_class);  
}

Camera *create_camera(Options &options, IConfiguration &config)
{
        const char *camera_class = get_camera_class(options, config);
        if (camera_class == 0)
                throw std::runtime_error("No camera class was defined in the options "
                                         "or in the configuration file.");
        
        JsonCpp camera_config;
        get_camera_config(camera_class, config, camera_config);
        
        Camera *camera = CameraFactory::create(camera_class, camera_config);
        if (camera == 0) {
                r_err("Failed to create the camera '%s'", camera_class);
                throw std::runtime_error("Failed to create the camera");
        }
        return camera;
}

const char *get_cnc_class(Options &options, IConfiguration &config)
{
        const char *cnc_class = options.cnc_class;
        if (cnc_class == 0) {
                try {
                        cnc_class = config.get("weeder").str("cnc");
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for weeder.cnc: %s", je.what());
                }
        }
        return cnc_class;
}

CNC *create_cnc(Options &options, IConfiguration &config)
{
        const char *cnc_class = get_cnc_class(options, config);
        if (cnc_class == 0)
                throw std::runtime_error("No CNC class was defined in the options "
                                         "or in the configuration file.");
        
        CNC *cnc = 0;
        
        if (rstreq(cnc_class, FakeCNC::ClassName)) {
                try {
                        cnc = new FakeCNC(config);
                } catch (JSONError &je) {
                        r_warn("Failed to configure FakeCNC: %s", je.what());
                }
                
        } else if (rstreq(cnc_class, RemoteCNC::ClassName)) {
                const char *name = options.cnc_name;
                if (name == 0)
                        name = "oquam";
                rpc_client = new RPCClient(name, "cnc");
                cnc = new RemoteCNC(rpc_client);
        }

        if (cnc == 0) {
                r_err("Failed to create the CNC '%s'", cnc_class);
                throw std::runtime_error("Failed to create the CNC");
        }
        
        return cnc;
}

void init_cnc_range(CNC *cnc, CNCRange &range)
{
        bool success = false;
        for (int attempt = 1; attempt <= 10; attempt++) {
                if (cnc->get_range(range)) {
                        r_debug("init_cnc_range: [%f, %f],[%f, %f],[%f, %f]",
                                range._x[0], range._x[1],
                                range._y[0], range._y[1],
                                range._z[0], range._z[1]);
                        success = true;
                        break; 
                }
                clock_sleep(1.0);
        }
        if (!success) {
                r_err("Failed to obtain the CNC range.");
                throw std::runtime_error("Failed to obtain the CNC range.");
        }
}

int main(int argc, char** argv)
{
        int retval = 1;
        Options options;
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        
        try {
                r_debug("Weeder: Using configuration file: '%s'", options.config_file);
                ConfigurationFile config(options.config_file);

                // Instantiate the camera
                Camera *camera = create_camera(options, config);
                
                // Instantiate the CNC
                CNC *cnc = create_cnc(options, config);

                CNCRange range;
                init_cnc_range(cnc, range);
                               
                Pipeline pipeline(range, config.get());
                
                DebugWeedingSession session(options.output_directory, "weeder");
                
                RoverWeeder weeder(&config, camera, &pipeline, cnc, range, session);
                
                RPCServer weeder_server(weeder,
                                        options.server_name,
                                        options.weeder_topic);
                
                // Make the camera accessible over HTTP 
                CameraServer camera_server(*camera, options.server_name,
                                           options.camera_topic);
                while (!app_quit())
                        clock_sleep(0.1);

                retval = 0;
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        if (cnc)
                delete cnc;
        if (rpc_client)
                delete rpc_client;
        if (camera)
                delete camera;
                
        
        return retval;
}

