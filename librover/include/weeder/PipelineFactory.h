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

#ifndef __ROMI_PIPELINE_FACTORY_H
#define __ROMI_PIPELINE_FACTORY_H

#include <memory>
#include <JsonCpp.h>
#include <cv/IImageCropper.h>
#include <configuration/GetOpt.h>
#include "IPipeline.h"
#include "IImageSegmentation.h"
#include "IConnectedComponents.h"
#include "IPathPlanner.h"
#include "IPipeline.h"

namespace romi {

        class IImageCropper;
        class IImageSegmentation;
        class IPathPlanner;
        class IPipeline;
        
        class PipelineFactory
        {
        protected:
                std::unique_ptr<IPipeline> _pipeline;
                
                std::unique_ptr<IImageCropper>
                build_cropper(CNCRange &range, JsonCpp weeder);

                std::unique_ptr<IImageSegmentation>
                build_segmentation(JsonCpp weeder);
                
                std::unique_ptr<IImageSegmentation>
                build_segmentation(const std::string& name, JsonCpp& weeder_props);
                
                std::unique_ptr<IConnectedComponents>
                build_connected_components(romi::GetOpt& options);
                
                
                std::unique_ptr<IPathPlanner> build_planner(JsonCpp weeder);
                
                std::unique_ptr<IPathPlanner> build_planner(const std::string& name,
                                                            JsonCpp& properties);
                
        public:
                PipelineFactory() : _pipeline() {}
                virtual ~PipelineFactory() = default;
                
                IPipeline& build(CNCRange &range, JsonCpp& config, romi::GetOpt& options);
        };
}

#endif // __ROMI_PIPELINE_FACTORY_H
