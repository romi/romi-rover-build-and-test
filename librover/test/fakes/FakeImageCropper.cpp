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

#include "FakeImageCropper.h"

namespace romi {
        
        FakeImageCropper::FakeImageCropper(Image& cropped_image, double meters_to_pixels)
                : cropped_image_(),
                  meters_to_pixels_(meters_to_pixels)
        {
                cropped_image_ = cropped_image;
        }
        
        double FakeImageCropper::map_meters_to_pixels(double meters)
        {
                return meters_to_pixels_ * meters;
        }
                
        bool FakeImageCropper::crop(ISession &session, Image &camera_image,
                                    double tool_diameter, Image &cropped_image)
        {
                (void) session;
                (void) camera_image;
                (void) tool_diameter;
                cropped_image = cropped_image_;
                return true;
        }
}
