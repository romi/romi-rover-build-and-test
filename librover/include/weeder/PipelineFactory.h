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
#include "IPipeline.h"
#include "cv/IImageCropper.h"
#include "IImageSegmentation.h"
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
                std::unique_ptr<IImageCropper> _cropper;
                std::unique_ptr<IImageSegmentation> _segmentation;
                std::unique_ptr<IPathPlanner> _planner;
                std::unique_ptr<IPipeline> _pipeline;
                
                void build_cropper(CNCRange &range, JsonCpp weeder);
                void build_segmentation(JsonCpp weeder);
                void build_planner(JsonCpp weeder);
                void build_planner(const std::string& name, JsonCpp& properties);
                
        public:
                PipelineFactory() : _cropper(), _segmentation(), _planner(), _pipeline() {}
                virtual ~PipelineFactory() = default;
                
                IPipeline& build(CNCRange &range, JsonCpp& config);
        };
}

#endif // __ROMI_PIPELINE_FACTORY_H
