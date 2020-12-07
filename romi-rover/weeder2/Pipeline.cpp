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

#include "Pipeline.h"
#include "PathPlannerFactory.h"
#include "ImageCropperFactory.h"
#include "ImageSegmentationFactory.h"

namespace romi {
        
        Pipeline::~Pipeline()
        {
                if (_cropper)
                        delete _cropper;
                if (_segmentation)
                        delete _segmentation;
                if (_planner)
                        delete _planner;
        }
        
        void Pipeline::build_cropper(CNCRange &range, JSON weeder)
        {
                const char *name = weeder.str("cropper");
                JSON properties = weeder.get(name);                
                _cropper = ImageCropperFactory::create(name, range, properties);
        }
        
        void Pipeline::build_segmentation(JSON weeder)
        {
                const char *name = weeder.str("segmentation");
                JSON properties = weeder.get(name);                
                _segmentation = ImageSegmentationFactory::create(name, properties);
        }
        
        void Pipeline::build_planner(JSON weeder)
        {
                const char *name = weeder.str("path");
                JSON properties = weeder.get(name);                
                _planner = PathPlannerFactory::create(name, properties);
        }
        
        void Pipeline::build(CNCRange &range, JSON config)
        {
                JSON weeder = config.get("weeder");
                build_cropper(range, weeder);
                build_segmentation(weeder);
                build_planner(weeder);
        }
        
        bool Pipeline::run(IFolder *session,
                           Image &camera,
                           double tool_diameter,
                           Path &path)
        {
                bool success = false;
                
                if (_cropper && _segmentation && _planner) {
                
                        double meters_to_pixels = _cropper->map_meters_to_pixels(1.0);
                
                        Image crop;
                        _cropper->crop(session, camera, tool_diameter, crop);
                        session->store_png("crop", crop);

                        Image mask;
                        _segmentation->segment(session, crop, mask);
                        session->store_png("mask", mask);

                        success = _planner->trace_path(session,
                                                       mask,
                                                       tool_diameter,
                                                       // image scaled 1/3
                                                       //meters_to_pixels/3,
                                                       // image not scaled
                                                       meters_to_pixels,
                                                       path);
                        if (success)
                                session->print_path(path);
                                
                } else {
                        r_err("Pipeline::run: not initialized");
                }

                return success;
        }
}

