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
// TBD: Override this class with FakeFakePipelineFactory (In romifakes library)
// TBD: Move all the fake code to FakeFakePipelineFactory.
// In build_pipeline build fake if it's in config OR options. If not call base class and build real class.

//#include <constraintsolver/GConstraintSolver.h>
//#include <cv/ImageCropper.h>
#include <cv/ImageIO.h>
#include <Pipeline.h>
#include "FakePipelineFactory.h"
#include "FakeConnectedComponents.h"
#include "FakeImageCropper.h"
#include "FakeSVMSegmentation.h"

namespace romi {

        std::unique_ptr<IImageCropper>
        FakePipelineFactory::build_cropper(CNCRange &range, JsonCpp& weeder, romi::GetOpt& options)
        {
            std::string path = options.get_value("cropper");
            if (path.empty()) {
                return PipelineFactory::build_cropper(range, weeder);
            } else {
                Image image;
                std::string meters_to_pixels = options.get_value("meters_to_pixels");
                if (!romi::ImageIO::load(image, path.c_str()))
                    throw std::runtime_error("Couldn't load components image");
                return std::make_unique<FakeImageCropper>(image, std::stod(meters_to_pixels));
            }
        }

        std::unique_ptr<IImageSegmentation>
        FakePipelineFactory::build_segmentation(JsonCpp& weeder, romi::GetOpt& options)
        {
            std::string path = options.get_value("mask");
            if (path.empty()) {
                return PipelineFactory::build_segmentation(weeder);
            } else {
                Image image;
                if (!romi::ImageIO::load(image, path.c_str()))
                    throw std::runtime_error("Couldn't load components image");
                return std::make_unique<FakeSVMSegmentation>(image);
            }
        }

        std::unique_ptr<IConnectedComponents>
        FakePipelineFactory::build_connected_components(romi::GetOpt& options)
        {
                std::string path = options.get_value("components");
                if (path.empty()) {
                    return PipelineFactory::build_connected_components();
                } else {
                        Image image;
                        if (!romi::ImageIO::load(image, path.c_str()))
                                throw std::runtime_error("Couldn't load components image");
                        return std::make_unique<FakeConnectedComponents>(image);
                }
        }

        IPipeline& FakePipelineFactory::build(CNCRange &range, JsonCpp& config,
                                          romi::GetOpt& options)
        {
                JsonCpp weeder = config["weeder"];

                auto cropper = build_cropper(range, weeder, options);
                auto connected_components = build_connected_components(options);
                
                auto segmentation = build_segmentation(weeder, options);
                auto planner = build_planner(weeder);
                
                _pipeline = std::make_unique<Pipeline>(cropper, segmentation,
                                                       connected_components, planner);
                return *_pipeline;
        }

}
