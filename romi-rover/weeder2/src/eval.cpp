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

#include "Linux.h"
#include <rover/RoverOptions.h>
#include <FileCamera.h>
#include <weeder/Weeder.h>
#include <weeder/PipelineFactory.h>
#include <fake/FakeCNC.h>
#include <oquam/Oquam.h>
#include <oquam/StepperSettings.h>
#include <oquam/FakeCNCController.h>
#include "data_provider/RomiDeviceData.h"
#include "data_provider/SoftwareVersion.h"
#include "weeder/Session.h"
#include "data_provider/Gps.h"
#include "data_provider/GpsLocationProvider.h"
//#include "RoverConfiguration.h"

#include "Clock.h"
#include "ClockAccessor.h"

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

int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        r_log_init();
        r_log_set_app("eval");
        std::signal(SIGSEGV, SignalHandler);
        std::signal(SIGINT, SignalHandler);
        
        try {
                romi::RoverOptions options;
                options.parse(argc, argv);
                options.exit_if_help_requested();
                
                JsonCpp config_file = JsonCpp::load(options.get_config_file().c_str());

                //RoverConfiguration configuration(options, config_file);
                
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
                JsonCpp oquam_config = config_file["oquam"];
                JsonCpp range_data = oquam_config["cnc-range"];
                double slice_duration = (double) oquam_config["path-slice-duration"];
                double maximum_deviation = (double) oquam_config["path-maximum-deviation"];
                JsonCpp stepper_data = oquam_config["stepper-settings"];
                
                romi::CNCRange range(range_data);                
                romi::StepperSettings stepper_settings(stepper_data);
                romi::FakeCNCController controller = romi::FakeCNCController();
                romi::AxisIndex homing[3] = { romi::kAxisX, romi::kAxisY, romi::kNoAxis };
                romi::OquamSettings oquam_settings(range,
                                                   stepper_settings.maximum_speed,
                                                   stepper_settings.maximum_acceleration,
                                                   stepper_settings.steps_per_meter,
                                                   maximum_deviation,
                                                   slice_duration,
                                                   homing);
                romi::Oquam oquam(controller, oquam_settings, session);

                // File camera
                romi::FileCamera camera(options.get_value("camera-image"));

                // Weeding pipeline
                romi::PipelineFactory factory;
                romi::IPipeline& pipeline = factory.build(range, config_file);

                // Weeder
                double z0 = (double) config_file["weeder"]["z0"];
                double speed = (double) config_file["weeder"]["speed"];
                romi::Weeder weeder(camera, pipeline, oquam, z0, speed, session);
                
                weeder.hoe();
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

