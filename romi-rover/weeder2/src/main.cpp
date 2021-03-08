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
#include <RPCClient.h>

#include <rover/RoverOptions.h>
#include <DebugWeedingSession.h>
#include <FileCamera.h>
#include <USBCamera.h>
#include <CameraServer.h>
#include <weeder/Weeder.h>
#include <weeder/PipelineFactory.h>
#include <rpc/WeederAdaptor.h>
#include <rpc/RemoteCNC.h>
#include <fake/FakeCNC.h>

using namespace romi;
using namespace rcom;

// TBD: Move scope.
static RPCClient *rpc_client = nullptr;

const char *get_config_file(Options& options)
{
        const char *file = options.get_value(RoverOptions::config);
        if (file == nullptr) {
                throw std::runtime_error("No configuration file was given (can't run without one...).");
        }
        return file;
}

const char *get_camera_image_in_config(JsonCpp &config)
{
        try {
                return (const char *)config["weeder"]["file-camera"]["image"];
                
        } catch (JSONError& je) {
                r_err("get_camera_image_in_config: Failed to get value "
                      "of weeder.file-camera.image");
                throw std::runtime_error("Missing image file for camera");
        }
}

const char *get_camera_image(Options &options, JsonCpp &config)
{
        const char *filename = options.get_value(RoverOptions::camera_image);
        if (filename == nullptr)
                filename = get_camera_image_in_config(config);
        return filename;
}

ICamera *instantiate_file_camera(Options &options, JsonCpp &config)
{
        return new FileCamera(get_camera_image(options, config));
}

const char *get_camera_device_in_config(JsonCpp &config)
{
        try {
                return (const char *)config["ports"]["usb-camera"]["port"];
                
        } catch (JSONError& je) {
                r_err("get_camera_device_in_config: Failed to get value "
                      "of ports.usb-camera.port");
                throw std::runtime_error("Missing device name for camera in config");
        }
}

const char *get_camera_device(Options &options, JsonCpp &config)
{
        const char *device = options.get_value("weeder-camera-device");
        if (device == nullptr)
                device = get_camera_device_in_config(config);
        return device;
}

ICamera *instantiate_usb_camera(Options &options, JsonCpp &config)
{
        try {
                const char *device = get_camera_device(options, config);
                double width = (double) config["weeder"]["usb-camera"]["width"];
                double height = (double) config["weeder"]["usb-camera"]["height"];
                return new USBCamera(device, (size_t) width, (size_t) height);
                
        } catch (JSONError& je) {
                r_err("instantiate_usb_camera: Failed to get width and height of "
                      "the camera in the config");
                throw std::runtime_error("Missing width or heidth for the USB camera");
        }
}

ICamera *instantiate_camera(const char *camera_class, Options &options, JsonCpp &config)
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

const char *get_camera_class(__attribute__((unused))Options &options, JsonCpp &config)
{
        try {
                return (const char *) config["weeder"]["camera-classname"];
                
        } catch (JSONError& je) {
                r_err("get_camera_class: Failed to get value "
                      "of weeder.camera-classname");
                throw std::runtime_error("Missing camera class in config");
        }
}

ICamera *create_camera(Options &options, JsonCpp &config)
{
        const char *camera_class = get_camera_class(options, config);
        return instantiate_camera(camera_class, options, config);
}

const char *get_cnc_class(__attribute__((unused))Options &options, JsonCpp &config)
{
        try {
                return (const char *) config["weeder"]["cnc-classname"];
                
        } catch (JSONError& je) {
                r_err("get_cnc_class: Failed to get value "
                      "of weeder.cnc-classname");
                throw std::runtime_error("Missing CNC class in config");
        }
}

ICNC *create_cnc(Options &options, JsonCpp &config)
{
        const char *cnc_class = get_cnc_class(options, config);
        ICNC *localcnc = nullptr;
        
        if (rstreq(cnc_class, FakeCNC::ClassName)) {
                try {
                        JsonCpp range_data = config["oquam"]["cnc-range"];
                        localcnc = new FakeCNC(range_data);
                        
                } catch (JSONError &je) {
                        r_warn("Failed to configure FakeCNC: %s", je.what());
                }
                
        } else if (rstreq(cnc_class, RemoteCNC::ClassName)) {
                rpc_client = new RPCClient("oquam", "cnc", 60.0);
                localcnc = new RemoteCNC(*rpc_client);
        }

        if (localcnc == nullptr) {
                r_err("Failed to create the CNC '%s'", cnc_class);
                throw std::runtime_error("Failed to create the CNC");
        }
        
        return localcnc;
}

int main(int argc, char** argv)
{
        static ICNC *cnc = nullptr;
//        static RPCClient *rpc_client = nullptr;
        static ICamera *camera = nullptr;

        int retval = 1;

        RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();
        
        app_init(&argc, argv);
        app_set_name("weeder");
        
        try {
                const char *config_file = get_config_file(options);
                r_info("IWeeder: Using configuration file: '%s'", config_file);
                JsonCpp config = JsonCpp::load(config_file);

                // Instantiate the camera
                camera = create_camera(options, config);
                
                // Instantiate the ICNC
                cnc = create_cnc(options, config);

                JsonCpp config_range = config["oquam"]["cnc-range"];
                
                CNCRange range;
                if (!cnc->get_range(range)) 
                        throw std::runtime_error("Failed to get the ICNC range");
                               
                PipelineFactory factory;
                IPipeline& pipeline = factory.build(range, config);
                
                DebugWeedingSession session(options.get_value("session-directory"),
                                            "weeder");

                
                double z0 = (double) config["weeder"]["z0"];
                double speed = (double) config["weeder"]["speed"];
                Weeder weeder(*camera, pipeline, *cnc, z0, speed, session);
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


        delete cnc;

        delete rpc_client;

        delete camera;
                
        
        return retval;
}

