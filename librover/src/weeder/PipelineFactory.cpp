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

// TBD: NO FAKE CODE IN PRODUCTION CODE!
// TBD: Override this class with FakePipelineFactory (In romifakes library)
// TBD: Move all the fake code to FakePipelineFactory.
// In build_pipeline build fake if it's in config OR options. If not call base class and build real class.

#include <constraintsolver/GConstraintSolver.h>
#include <cv/ImageCropper.h>

#include "weeder/PipelineFactory.h"
#include "weeder/Pipeline.h"
#include "weeder/ConnectedComponents.h"
#include "svm/SVMSegmentation.h"
#include "unet/PythonUnet.h"
#include "unet/PythonSVM.h"
#include "som/SOM.h"
#include "quincunx/Quincunx.h"

namespace romi {
        
        std::unique_ptr<IImageCropper>
        PipelineFactory::build_cropper(CNCRange &range, JsonCpp& weeder)
        {
                const char *name = (const char *) weeder["cropper"];
                JsonCpp properties = weeder[name];
                return std::make_unique<ImageCropper>(range, properties);
        }
        
        std::unique_ptr<IImageSegmentation> 
        PipelineFactory::build_segmentation(JsonCpp& weeder)
        {
                const char *name = (const char *) weeder["segmentation"];
                return build_segmentation(name, weeder);
        }
        
        std::unique_ptr<IImageSegmentation> 
        PipelineFactory::build_segmentation(const std::string& name, JsonCpp& weeder)
        {
                if (name == kSVM) {
                        JsonCpp properties = weeder["svm"];                
                        return std::make_unique<SVMSegmentation>(properties);
                        
                } else if (name == kPythonUnet) {
                        return std::make_unique<PythonUnet>();
                        
                } else if (name == kPythonSVM) {
                        return std::make_unique<PythonSVM>();
                        
                } else {
                        r_err("Failed to find the segmentation class: %s", name.c_str());
                        throw std::runtime_error("Invalid segmentation class");
                }
        }
                
        std::unique_ptr<IConnectedComponents>
        PipelineFactory::build_connected_components()
        {
                    return std::make_unique<ConnectedComponents>();
        }
        
        std::unique_ptr<IPathPlanner>
        PipelineFactory::build_planner(JsonCpp& weeder)
        {
                std::string name = (const char *) weeder["path"];
                JsonCpp properties = weeder[name.c_str()];
                return build_planner(name, properties);
        }
        
        std::unique_ptr<IPathPlanner>
        PipelineFactory::build_planner(const std::string& name, JsonCpp& properties)
        {
                if (name == kQuincunx) {
                        return std::make_unique<Quincunx>(properties);
                } else if (name ==  kSOM) {
                        return std::make_unique<SOM>(properties);
                } else if (name ==  kORTools) {
                        return std::make_unique<GConstraintSolver>(properties);
                } else {
                        r_err("Failed to find the path planner class: %s", name.c_str());
                        throw std::runtime_error("Invalid path planner class");
                }
        }
        
        IPipeline& PipelineFactory::build(CNCRange &range, JsonCpp& config)
        {
                JsonCpp weeder = config["weeder"];

                auto cropper = build_cropper(range, weeder);
                auto connected_components = build_connected_components();
                
                auto segmentation = build_segmentation(weeder);
                auto planner = build_planner(weeder);
                
                _pipeline = std::make_unique<Pipeline>(cropper, segmentation,
                                                       connected_components, planner);
                return *_pipeline;
        }
}
