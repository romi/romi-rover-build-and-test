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
#include <syslog.h>

#include <Linux.h>
#include <Clock.h>
#include <ClockAccessor.h>
#include <RSerial.h>
#include <RomiSerialClient.h>
#include <rover/RoverOptions.h>
#include <camera/FileCamera.h>
#include <camera/USBCamera.h>
#include <rpc/RcomClient.h>
#include <rpc/RemoteCamera.h>
#include <weeder/Weeder.h>
#include <fake/FakeCNC.h>
#include <oquam/Oquam.h>
#include <oquam/StepperSettings.h>
#include <oquam/StepperController.h>
#include <oquam/FakeCNCController.h>
#include <data_provider/RomiDeviceData.h>
#include <data_provider/SoftwareVersion.h>
#include <session/Session.h>
#include <data_provider/Gps.h>
#include <data_provider/GpsLocationProvider.h>
#include <configuration/ConfigurationProvider.h>

#include <FakePipelineFactory.h>

void SignalHandler(int signal)
{
        if (signal == SIGSEGV){
                syslog(1, "rcom-registry segmentation fault");
                exit(signal);
        } else if (signal == SIGINT){
                r_info("Ctrl-C Quitting Application");
                perror("init_signal_handler");
        } else{
                r_err("Unknown signam received %d", signal);
        }
}

static const std::string cropper("cropper");
const std::string meters_to_pixels("meters-to-pixels");
const std::string components("components");
const std::string mask("mask");


static std::vector<romi::Option> eval_options = {
        { "help", false, nullptr,
          "Print help message" },

        // File paths
                
        { romi::RoverOptions::config, true, "config.json",
          "Path of the config file" },
                
        { romi::RoverOptions::session_directory, true, ".",
          "The session directory where the output "
          "files are stored (logs, images...)"},
                
        { romi::RoverOptions::camera_classname, true, romi::FileCamera::ClassName,
          "The classname of the camera to instanciate (default: file-camera)."},
                
        { romi::RoverOptions::camera_image, true, nullptr,
          "The path of the image file for the file camera."},

        { cropper.c_str(), true, nullptr,
          "The cropped image"},
        
        { meters_to_pixels.c_str(), true, "1000",
          "Meters to pixels conversion"},

        { components.c_str(), true, nullptr,
          "The connected components image "},
                
        { mask.c_str(), true, nullptr,
          "The mask "}
};


int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        r_log_init();
        r_log_set_app("eval");
        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        
        try {
                romi::GetOpt options(eval_options);
                options.parse(argc, argv);
                if (options.is_help_requested()) {
                        options.print_usage();
                        exit(0);
                }
 
                std::string path = options.get_value(romi::RoverOptions::config);
                if (path.empty())
                        throw std::runtime_error("No configuration file was given");

                JsonCpp config = JsonCpp::load(path.c_str());
                
                // Session
                rpp::Linux linux;
                romi::RomiDeviceData romiDeviceData;
                romi::SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<romi::ILocationProvider> locationPrivider
                        = std::make_unique<romi::GpsLocationProvider>(gps);
                std::string session_directory = options.get_value("session-directory");
                romi::Session session(linux, session_directory, romiDeviceData,
                                      softwareVersion, std::move(locationPrivider));
                r_info("Session directory is %s", session_directory.c_str());

                // CNC
                JsonCpp oquam_config = config["oquam"];
                JsonCpp range_data = oquam_config["cnc-range"];
                double slice_duration = (double) oquam_config["path-slice-duration"];
                double maximum_deviation = (double) oquam_config["path-maximum-deviation"];
                JsonCpp stepper_data = oquam_config["stepper-settings"];
                
                romi::CNCRange range(range_data);                
                romi::StepperSettings stepper_settings(stepper_data);

                
                r_info("main: Creating CNC controller");
                std::unique_ptr<romi::ICNCController> controller;
                if (true) {
                        const char *cnc_device = (const char *) config["ports"]["oquam"]["port"];
                        auto cnc_serial = romiserial::RomiSerialClient::create(cnc_device);
                        controller = std::make_unique<romi::StepperController>(cnc_serial);
                } else {
                        controller = std::make_unique<romi::FakeCNCController>();
                }
                romi::AxisIndex homing[3] = { romi::kAxisX, romi::kAxisY, romi::kNoAxis };
                romi::OquamSettings oquam_settings(range,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   1.0,
                                                   homing);
                romi::Oquam oquam(*controller, oquam_settings, session);

                // File camera
                //romi::FileCamera camera(options.get_value("camera-image"));

                // Camera
                // TBD: Use refactored functions. get_camera_class
                r_info("main: Creating camera");
                std::unique_ptr<romi::ICamera> camera;
                std::string camera_classname = get_camera_classname(options, config);
                
                if (camera_classname == romi::FileCamera::ClassName) {
                        std::string image_file = get_camera_image(options, config);
                        r_info("Loading image %s", image_file.c_str());
                        camera = std::make_unique<romi::FileCamera>(image_file);
                        
                } else if (camera_classname == romi::USBCamera::ClassName) {
                        std::string camera_device = get_camera_device(options, config);
                        double width = (double) config["weeder"]["usb-camera"]["width"];
                        double height = (double) config["weeder"]["usb-camera"]["height"];
                        camera = std::make_unique<romi::USBCamera>(camera_device, width, height);
                } else if (camera_classname == romi::RemoteCamera::ClassName) {
                        auto client = romi::RcomClient::create("camera", 10.0);
                        camera = std::make_unique<romi::RemoteCamera>(client);
                } else {
                        throw std::runtime_error("Unknown camera classname");
                }

                // Weeding pipeline
                romi::FakePipelineFactory factory;
                romi::IPipeline& pipeline = factory.build(range, config, options);

                // Weeder
                double z0 = (double) config["weeder"]["z0"];
                double speed = (double) config["weeder"]["speed"];
                romi::Weeder weeder(*camera, pipeline, oquam, z0, speed, session);
                
                weeder.hoe();
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

