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

#ifndef __ROMI_PIPELINE_H
#define __ROMI_PIPELINE_H

#include "JsonCpp.h"
#include "CNCRange.h"
#include "IPipeline.h"
#include "IPathPlanner.h"
#include "IImageCropper.h"
#include "IImageSegmentation.h"

namespace romi {
        
        class Pipeline : public IPipeline
        {
        protected:
                IImageCropper* _cropper;
                IImageSegmentation* _segmentation;
                IPathPlanner* _planner;

                void build_cropper(CNCRange &range, JsonCpp weeder);
                void build_segmentation(JsonCpp weeder);
                void build_planner(JsonCpp weeder);
                void build(CNCRange &range, JsonCpp weeder);
                
        public:
                 Pipeline(CNCRange &range, JsonCpp config)
                        : _cropper(0), _segmentation(0), _planner(0) {
                        build(range, config);
                }
                
                virtual ~Pipeline() override;
                
                bool run(IFolder *session, Image &camera,
                         double tool_diameter, Path &path) override;
        };
}

#endif // __ROMI_PIPELINE_H
