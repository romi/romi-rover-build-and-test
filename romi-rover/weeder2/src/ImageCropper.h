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

#ifndef __ROMI_IMAGE_CROPPER_H
#define __ROMI_IMAGE_CROPPER_H

#include "IImageCropper.h"

namespace romi {
        
        class ImageCropper : public IImageCropper
        {
        protected:
                CNCRange _range;
                int _x0;
                int _y0;
                int _width;
                int _height;

                int set_workspace(JsonCpp value);

        public:
                ImageCropper() : _x0(0), _y0(0), _width(0), _height(0) {} 
                virtual ~ImageCropper() override = default;
                
                void set_range(CNCRange &range) override {
                        _range = range;

                        r_debug("range: %f, %f", range._x[0], range._x[1]);
                        r_debug("_range: %f, %f", _range._x[0], _range._x[1]);
                }

                double map_meters_to_pixels(double meters) override;

                int set_parameter(const char *name, JsonCpp value) override;
                        
                void crop(IFolder &session,
                          Image &camera_image,
                          double tool_diameter,
                          Image &cropped_image) override;
        };
}

#endif // __ROMI_IMAGE_CROPPER_H
