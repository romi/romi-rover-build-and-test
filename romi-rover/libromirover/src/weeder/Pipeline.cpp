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

#include "weeder/Pipeline.h"

namespace romi {

        void Pipeline::crop_image(IFolder& session, Image& camera,
                                  double tool_diameter, Image& crop)
        {
                if (!_cropper.crop(session, camera, tool_diameter, crop)) {
                        throw std::runtime_error("Pipeline: crop failed");
                }
        }

        void Pipeline::segment_image(IFolder& session, Image& crop, Image& mask)
        {
                if (!_segmentation.segment(session, crop, mask)) {
                        throw std::runtime_error("Pipeline: segmentation failed");
                }
        }

        void Pipeline::trace_path(IFolder& session, Image& mask, double tool_diameter,
                                  double meters_to_pixels, Path& path)
        {
                if (!_planner.trace_path(session, mask, tool_diameter,
                                         meters_to_pixels, path)) {
                        throw std::runtime_error("Pipeline: path planner failed");
                }
        }

        void Pipeline::try_run(IFolder& session, Image& camera,
                               double tool_diameter, Path& path)
        {
                
                Image crop;
                crop_image(session, camera, tool_diameter, crop);
                session.store_png("crop", crop);

                Image mask;
                segment_image(session, crop, mask);
                session.store_png("mask", mask);

                double meters_to_pixels = _cropper.map_meters_to_pixels(1.0);
                // meters_to_pixels = meters_to_pixels / 3.0; // image scaled 1/3
                
                trace_path(session, mask, tool_diameter, meters_to_pixels, path);
                session.print_path(path);
        }

        bool Pipeline::run(IFolder& session, Image& camera,
                           double tool_diameter, Path& path)
        {
                bool success = false;
                try  {
                        try_run(session, camera, tool_diameter, path);
                        success = true;
                        
                } catch (std::runtime_error& e) {
                        r_err("Pipeline::run: try_run failed");
                }
                
                return success;
        }
}

