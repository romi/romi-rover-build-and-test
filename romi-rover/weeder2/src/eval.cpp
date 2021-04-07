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
#include <rcom.h>

#include "Linux.h"
#include <rover/RoverOptions.h>
#include <FileCamera.h>
#include <weeder/Weeder.h>
#include <weeder/PipelineFactory.h>
#include <fake/FakeCNC.h>
#include "data_provider/RomiDeviceData.h"
#include "data_provider/SoftwareVersion.h"
#include "weeder/Session.h"
#include "data_provider/Gps.h"
#include "data_provider/GpsLocationProvider.h"

#include "Clock.h"
#include "ClockAccessor.h"

using namespace romi;

int main(int argc, char** argv)
{
        std::shared_ptr<rpp::IClock> clock = std::make_shared<rpp::Clock>();
        rpp::ClockAccessor::SetInstance(clock);

        RoverOptions options;
        options.parse(argc, argv);
        
        app_init(&argc, argv);

        try {
                JsonCpp config = JsonCpp::load(options.get_value("config-file"));
                
                JsonCpp range_data = config["oquam"]["cnc-range"];
                FakeCNC cnc(range_data);
                
                CNCRange range;
                cnc.get_range(range);
                
                FileCamera camera(options.get_value("weeder-camera-image"));

                PipelineFactory factory;
                IPipeline& pipeline = factory.build(range, config);

                // Session
                rpp::Linux linux;
                RomiDeviceData romiDeviceData;
                SoftwareVersion softwareVersion;
                romi::Gps gps;
                std::unique_ptr<ILocationProvider> locationPrivider = std::make_unique<GpsLocationProvider>(gps);
                std::string session_directory = options.get_value("session-directory");
                romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));

                double z0 = (double) config["weeder"]["z0"];
                double speed = (double) config["weeder"]["speed"];
                Weeder weeder(camera, pipeline, cnc, z0, speed, session);
                
                weeder.hoe();
                
        } catch (std::exception& e) {
                r_err(e.what());
        }

        return 0;
}

