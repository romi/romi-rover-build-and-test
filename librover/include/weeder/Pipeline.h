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

#include <JsonCpp.h>
#include <MemBuffer.h>
#include <api/CNCRange.h>
#include <cv/IImageCropper.h>

#include "IPathPlanner.h"
#include "IImageSegmentation.h"
#include "IConnectedComponents.h"
#include "IPipeline.h"

namespace romi {
        
        class Pipeline : public IPipeline
        {
        protected:
                std::unique_ptr<IImageCropper> cropper_;
                std::unique_ptr<IImageSegmentation> segmentation_;
                std::unique_ptr<IConnectedComponents> connected_components_;
                std::unique_ptr<IPathPlanner> planner_;
                
                void create_mask(ISession& session, Image &crop, Image &mask);

                Path trace_path(ISession& session, Centers& centers, Image& mask);
                
                void crop_image(ISession& session, Image& camera,
                                double tool_diameter, Image& crop);

                void check_path(ISession& session, Image& mask, Path& path,
                                Path& result, size_t index);
                void check_segment(rpp::MemBuffer& buffer,
                                   Image& image, v3 start, v3 end,
                                   Path& path);
                std::vector<Path> try_run(ISession& session, Image& camera,
                                          double tool_diameter);

                void go_around(rpp::MemBuffer& buffer, Image& mask,
                               v3 start, v3 end, Path& path);

        public:
                Pipeline(std::unique_ptr<IImageCropper>& cropper,
                         std::unique_ptr<IImageSegmentation>& segmentation,
                         std::unique_ptr<IConnectedComponents>& connected_components,
                         std::unique_ptr<IPathPlanner>& planner);

                ~Pipeline() override = default;
                
                std::vector<Path> run(ISession& session, Image& camera,
                                      double tool_diameter) override;
        };
}

#endif // __ROMI_PIPELINE_H
