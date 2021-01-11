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
#include <stdexcept>
#include <string.h>

#include <rcom.h>
#include <RPCServer.h>
#include <RoverOptions.h>
#include <DebugWeedingSession.h>
#include <RPCClient.h>

#include "FakeCNC.h"
#include "FileCamera.h"
#include "USBCamera.h"
#include "CameraServer.h"
#include "RoverWeeder.h"
#include "weeder/PipelineFactory.h"
#include "rpc/WeederAdaptor.h"
#include "rpc/RemoteCNC.h"

using namespace romi;
using namespace rcom;

static CNC *cnc = 0;
static RPCClient *rpc_client = 0;
static Camera *camera = 0;

const char *get_camera_image(Options &options, JsonCpp &config)
{
        const char *filename = options.get_value("weeder-camera-image");
        if (filename == 0)
                throw std::runtime_error("Missing filename for camera image");
        return filename;
}

Camera *instantiate_file_camera(Options &options, JsonCpp &config)
{
        return new FileCamera(get_camera_image(options, config));
}

const char *get_camera_device_in_config(JsonCpp &config)
{
        try {
                return config["ports"]["usb-camera"]["port"];
                
        } catch (JSONError& je) {
                r_err("get_camera_device_in_config: Failed to get value "
                      "of ports.usb-camera.port");
                throw std::runtime_error("Missing device name for camera in config");
        }
}

const char *get_camera_device(Options &options, JsonCpp &config)
{
        const char *device = options.get_value("weeder-camera-device");
        if (device == 0)
                device = get_camera_device_in_config(config);
        return device;
}

Camera *instantiate_usb_camera(Options &options, JsonCpp &config)
{
        try {
                const char *device = get_camera_device(options, config);
                double width = config["weeder"]["usb-camera"]["width"];
                double height = config["weeder"]["usb-camera"]["height"];
                return new USBCamera(device, (size_t) width, (size_t) height);
                
        } catch (JSONError& je) {
                r_err("instantiate_usb_camera: Failed to get width and height of "
                      "the camera in the config");
                throw std::runtime_error("Missing width or heidth for the USB camera");
        }
}

Camera *instantiate_camera(const char *camera_class, Options &options, JsonCpp &config)
{
        if (rstreq(camera_class, FileCamera::ClassName)) {
                return instantiate_file_camera(options, config);

        } else if (rstreq(camera_class, USBCamera::ClassName)) {
                return instantiate_usb_camera(options, config);

        } else {
                r_err("instantiate_camera: Unknown camera classname: %s", camera_class);
                throw std::runtime_error("Unknown camera classname");
        }
}

const char *get_camera_class(Options &options, JsonCpp &config)
{
        const char *camera_class = options.get_value("weeder-camera-classname");
        if (camera_class == 0) {
                try {
                        camera_class = config["weeder"]["camera-classname"];
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for "
                               "weeder.camera-classname: %s", je.what());
                }
        }
        return camera_class;
}

Camera *create_camera(Options &options, JsonCpp &config)
{
        const char *camera_class = get_camera_class(options, config);
        if (camera_class == 0)
                throw std::runtime_error("No camera class was defined in the options "
                                         "or in the configuration file.");
        return instantiate_camera(camera_class, options, config);
}

const char *get_cnc_class(Options &options, JsonCpp &config)
{
        const char *cnc_class = options.get_value("weeder-cnc-classname");
        if (cnc_class == 0) {
                try {
                        cnc_class = config["weeder"]["cnc-classname"];
                } catch (JSONError &je) {
                        r_warn("Failed to get the value for weeder.cnc: %s", je.what());
                }
        }
        return cnc_class;
}

CNC *create_cnc(Options &options, JsonCpp &config)
{
        const char *cnc_class = get_cnc_class(options, config);
        if (cnc_class == 0)
                throw std::runtime_error("No CNC class was defined in the options "
                                         "or in the configuration file.");
        
        CNC *cnc = 0;
        
        if (rstreq(cnc_class, FakeCNC::ClassName)) {
                try {
                        JsonCpp range_data = config["oquam"]["cnc-range"];
                        cnc = new FakeCNC(range_data);
                        
                } catch (JSONError &je) {
                        r_warn("Failed to configure FakeCNC: %s", je.what());
                }
                
        } else if (rstreq(cnc_class, RemoteCNC::ClassName)) {
                rpc_client = new RPCClient("oquam", "cnc", 60.0);
                cnc = new RemoteCNC(*rpc_client);
        }

        if (cnc == 0) {
                r_err("Failed to create the CNC '%s'", cnc_class);
                throw std::runtime_error("Failed to create the CNC");
        }
        
        return cnc;
}

int main(int argc, char** argv)
{
        int retval = 1;

        GetOpt options(rover_options, rover_options_length);
        options.parse(argc, argv);
        
        app_init(&argc, argv);
        app_set_name("weeder");
        
        try {
                r_debug("Weeder: Using configuration file: '%s'",
                        options.get_value("config"));
                
                JsonCpp config = JsonCpp::load(options.get_value("config"));

                // Instantiate the camera
                camera = create_camera(options, config);
                
                // Instantiate the CNC
                cnc = create_cnc(options, config);

                JsonCpp config_range = config["oquam"]["cnc-range"];
                
                CNCRange range;
                if (!cnc->get_range(range)) 
                        throw std::runtime_error("Failed to get the CNC range");
                               
                PipelineFactory factory;
                IPipeline& pipeline = factory.build(range, config);
                
                DebugWeedingSession session(options.get_value("session-directory"),
                                            "weeder");

                
                double z0 = config["weeder"]["z0"];
                double speed = config["weeder"]["speed"];
                RoverWeeder weeder(*camera, pipeline, *cnc, z0, speed, session);
                WeederAdaptor adaptor(weeder);
                RPCServer weeder_server(adaptor, "weeder", "weeder");
                
                // Make the camera accessible over HTTP 
                CameraServer camera_server(*camera, "weeder", "topcam");
                
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

