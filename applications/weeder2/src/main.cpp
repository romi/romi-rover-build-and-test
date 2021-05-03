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
#include <atomic>
#include <syslog.h>

#include <rpc/RcomServer.h>
#include <rpc/RcomClient.h>
#include <rpc/RemoteCamera.h>

#include "configuration/ConfigurationProvider.h"
#include <rover/RoverOptions.h>
#include <camera/FileCamera.h>
#include <camera/USBCamera.h>
#include <weeder/Weeder.h>
#include <weeder/PipelineFactory.h>
#include <rpc/WeederAdaptor.h>
#include <rpc/RemoteCNC.h>
#include <fake/FakeCNC.h>

#include "Linux.h"
#include "data_provider/RomiDeviceData.h"
#include "data_provider/SoftwareVersion.h"
#include "session/Session.h"
#include "data_provider/Gps.h"
#include "data_provider/GpsLocationProvider.h"
#include "Clock.h"
#include "ClockAccessor.h"

std::atomic<bool> quit(false);

std::shared_ptr<romi::ICamera> instantiate_file_camera(romi::IOptions &options,
                                                       JsonCpp &config)
{
        return std::make_shared<romi::FileCamera>(get_camera_image(options, config));
}


// TBD: This is duplicate code but the device is different.
// Should make it common.
std::string get_weeder_camera_device(romi::IOptions &options, JsonCpp &config)
{
        std::string device = options.get_value("weeder-camera-device");
        if (device.empty())
                device = romi::get_camera_device_in_config(config);
        return device;
}

std::shared_ptr<romi::ICamera> instantiate_usb_camera(romi::IOptions &options,
                                                      JsonCpp &config)
{
        try {
                std::string device = get_weeder_camera_device(options, config);
                double width = (double) config["weeder"]["usb-camera"]["width"];
                double height = (double) config["weeder"]["usb-camera"]["height"];
                return std::make_shared<romi::USBCamera>(device, (size_t) width,
                                                         (size_t) height);
                
        } catch (JSONError& je) {
                r_err("instantiate_usb_camera: Failed to get width and height of "
                      "the camera in the config");
                throw std::runtime_error("Missing width or heidth for the USB camera");
        }
}

std::shared_ptr<romi::ICamera> instantiate_remote_camera(romi::IOptions &options,
                                                         JsonCpp &config)
{
        (void) options;
        (void) config;
        
        try {
                auto client = romi::RcomClient::create("camera", 10.0);
                return std::make_shared<romi::RemoteCamera>(client);
                
        } catch (std::exception& e) {
                r_err("instantiate_remote_camera: %s", e.what());
                throw std::runtime_error("Create remote camera failed");
        }
}

std::shared_ptr<romi::ICamera> instantiate_camera(const std::string& camera_class,
                                                  romi::IOptions &options, JsonCpp &config)
{
        if (camera_class == romi::FileCamera::ClassName) {
                return instantiate_file_camera(options, config);

        } else if (camera_class == romi::USBCamera::ClassName) {
                return instantiate_usb_camera(options, config);

        } else if (camera_class == romi::RemoteCamera::ClassName) {
                return instantiate_remote_camera(options, config);

        } else {
                r_err("instantiate_camera: Unknown camera classname: %s",
                      camera_class.c_str());
                throw std::runtime_error("Unknown camera classname");
        }
}

std::string get_camera_class(romi::IOptions &options, JsonCpp &config)
{
        (void) options;
        std::string camera_class;
        try {
                auto cclass = (const char *) config["weeder"]["camera-classname"];
                if (cclass)
                        camera_class = cclass;
                
        } catch (JSONError& je) {
                r_err("get_camera_class: Failed to get value "
                      "of weeder.camera-classname");
                throw std::runtime_error("Missing camera class in config");
        }
        return camera_class;
}

std::shared_ptr<romi::ICamera> create_camera(romi::IOptions &options, JsonCpp &config)
{
        std::string camera_class = get_camera_class(options, config);
        return instantiate_camera(camera_class, options, config);
}

std::string get_cnc_class(romi::IOptions &options, JsonCpp &config)
{
        (void) options;
        try {
                return (const char *) config["weeder"]["cnc-classname"];
                
        } catch (JSONError& je) {
                r_err("get_cnc_class: Failed to get value "
                      "of weeder.cnc-classname");
                throw std::runtime_error("Missing CNC class in config");
        }
}

std::shared_ptr<romi::ICNC> create_cnc(romi::IOptions &options, JsonCpp &config)
{
        std::string cnc_class = get_cnc_class(options, config);
        std::shared_ptr<romi::ICNC> localcnc = nullptr;
        
        if (cnc_class == romi::FakeCNC::ClassName) {
                try {
                        JsonCpp range_data = config["oquam"]["cnc-range"];
                        localcnc = std::make_shared<romi::FakeCNC>(range_data);
                        
                } catch (JSONError &je) {
                        r_warn("Failed to configure FakeCNC: %s", je.what());
                }
                
        } else if (cnc_class == romi::RemoteCNC::ClassName) {
                auto client = romi::RcomClient::create("cnc", 60.0);
                localcnc = std::make_shared<romi::RemoteCNC>(client);
        }

        if (localcnc == nullptr) {
                r_err("Failed to create the CNC '%s'", cnc_class.c_str());
                throw std::runtime_error("Failed to create the CNC");
        }
        
        return localcnc;
}

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "rcom-registry segmentation fault");
                exit(signal);
        }
        else if (signal == SIGINT){
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
                quit = true;
        }
        else{
                r_err("Unknown signam received %d", signal);
        }
}

int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);
        std::shared_ptr<romi::ICNC> cnc = nullptr;
        std::shared_ptr<romi::ICamera> camera = nullptr;

        int retval = 1;

        romi::RoverOptions options;
        options.parse(argc, argv);
        options.exit_if_help_requested();

        r_log_init();
        r_log_set_app("weeder");

        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        
        try {
                std::string config_file = options.get_config_file();
                r_info("Navigation: Using configuration file: '%s'", config_file.c_str());
                JsonCpp config = JsonCpp::load(config_file.c_str());

                // Instantiate the camera
                camera = create_camera(options, config);
                
                // Instantiate the ICNC
                cnc = create_cnc(options, config);

                JsonCpp config_range = config["oquam"]["cnc-range"];

                romi::CNCRange range;
                if (!cnc->get_range(range)) 
                        throw std::runtime_error("Failed to get the ICNC range");

                romi::PipelineFactory factory;
                romi::IPipeline& pipeline = factory.build(range, config);

                rpp::Linux linux;
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationProvider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = options.get_value("session-directory");
                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationProvider));

                double z0 = (double) config["weeder"]["z0"];
                double speed = (double) config["weeder"]["speed"];
                romi::Weeder weeder(*camera, pipeline, *cnc, z0, speed, session);
                romi::WeederAdaptor adaptor(weeder);
                
                auto weeder_server = romi::RcomServer::create("weeder", adaptor);

                while (!quit) {
                        weeder_server->handle_events();
                        clock->sleep(0.2);
                }

                retval = 0;
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return retval;
}

