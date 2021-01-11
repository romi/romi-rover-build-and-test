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

#include <stdexcept>
#include "weeder/ImageCropper.h"

namespace romi {

        ImageCropper::ImageCropper(CNCRange& range, JsonCpp& properties)
                : _range(range), _x0(0), _y0(0), _width(0), _height(0)
        {
                try {
                        set_workspace(properties["workspace"]);
                        
                } catch (JSONError& je) {
                        r_err("ImageCropper: Failed to parse the workspace dimensions: %s",
                              je.what());
                        throw std::runtime_error("ImageCropper: bad config");
                }
        } 

        void ImageCropper::assert_workspace_dimensions()
        {
                if ((_x0 < 0) || (_x0 > 10000)
                    || (_y0 < 0) || (_y0 > 10000)
                    || (_width < 0) || (_width > 10000)
                    || (_height < 0) || (_height > 10000)) {
                        r_err("ImageCropper: Invalid workspace values: "
                              "workspace: x0 %d, y0 %d, width %d px, height %d px", 
                              _x0, _y0, _width, _height);
                        throw std::runtime_error("ImageCropper: bad workspace dimensions");
                }
        }
        
        void ImageCropper::set_workspace(JsonCpp w)
        {
                _x0 = (int) w.num(0);
                _y0 = (int) w.num(1);
                _width = (int) w.num(2);
                _height = (int) w.num(3);
                assert_workspace_dimensions();

                r_debug("workspace: x0 %d, y0 %d, width %d px, height %d px", 
                        _x0, _y0, _width, _height);
        }

        double ImageCropper::map_meters_to_pixels(double meters)
        {
                printf("x[0]=%f, x[1]=%f\n", _range.min.x(), _range.max.x());
                v3 dimensions = _range.dimensions();
                double meters_to_pixels = (double) _width / dimensions.x();
                return meters * meters_to_pixels;
        }

        bool ImageCropper::crop(IFolder &session,
                                Image &camera,
                                double tool_diameter,
                                Image &out)
        {
                bool success = false;
                double diameter = map_meters_to_pixels(tool_diameter);
                size_t border = (size_t) (diameter / 2.0);
                ssize_t x0 = _x0 - border;
                size_t width = _width + 2 * border;
                ssize_t y0 = camera.height() - _y0 - _height - border;
                size_t height = _height + 2 * border;
                
                session.store("camera", camera);

                if (x0 < 0) {
                        r_err("ImageCropper::crop: camera position is not good: "
                              "the bottom is cut off");
                } else if (width > camera.width()) {
                        r_err("ImageCropper::crop: camera image width too small");
                } else if (y0 < 0) {
                        r_err("ImageCropper::crop: camera position is not good: "
                              "the top is cut off");
                } else if (height > camera.height()) {
                        r_err("ImageCropper::crop: camera image height too small");
                                
                } else if (0) {
                        
                        // With scaling of factor 3
                        Image cropped;
                        camera.crop(x0, y0, width, height, cropped);
                        session.store("cropped_debug", cropped);

                        cropped.scale(3, out);
                        session.store("scaled_debug", out);
                        success = true;
                                
                } else {
                                
                        // Without scaling
                        camera.crop(x0, y0, width, height, out);
                        session.store("cropped_debug", out);
                        success = true;
                }

                return success;
        }
}
