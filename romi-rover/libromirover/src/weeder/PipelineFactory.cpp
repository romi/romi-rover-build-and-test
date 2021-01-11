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

#include "weeder/PipelineFactory.h"
#include "weeder/Pipeline.h"
#include "weeder/ImageCropper.h"

#include "svm/SVMSegmentation.h"
#include "som/SOM.h"
#include "quincunx/Quincunx.h"

namespace romi {
        
        void PipelineFactory::build_cropper(CNCRange &range, JsonCpp weeder)
        {
                const char *name = weeder["cropper"];
                JsonCpp properties = weeder[name];
                _cropper = std::make_unique<ImageCropper>(range, properties);
        }
        
        void PipelineFactory::build_segmentation(JsonCpp weeder)
        {
                const char *name = weeder["segmentation"];
                JsonCpp properties = weeder[name];                
                _segmentation = std::make_unique<SVMSegmentation>(properties);
        }
        
        void PipelineFactory::build_planner(JsonCpp weeder)
        {
                const char *name = weeder["path"];
                JsonCpp properties = weeder[name];                
                build_planner(name, properties);
        }
        
        void PipelineFactory::build_planner(const char *name, JsonCpp& properties)
        {
                if (rstreq(name, "quincunx")) {
                        _planner = std::make_unique<Quincunx>(properties);
                } else if (rstreq(name, "som")) {
                        _planner = std::make_unique<SOM>(properties);
                } else {
                        r_err("Failed to find the path planner class: %s", name);
                        throw std::runtime_error("Invalid path planner class");
                }
        }
        
        IPipeline& PipelineFactory::build(CNCRange &range, JsonCpp& config)
        {
                JsonCpp weeder = config["weeder"];
                build_cropper(range, weeder);
                build_segmentation(weeder);
                build_planner(weeder);
                _pipeline = std::make_unique<Pipeline>(*_cropper,
                                                       *_segmentation,
                                                       *_planner);
                return *_pipeline;
        }
}
