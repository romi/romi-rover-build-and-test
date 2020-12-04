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
#include "ImageCropper.h"

namespace romi {

        int ImageCropper::set_workspace(JSON w)
        {
                int r = -1;
                double x0 = w.num(0);
                double y0 = w.num(1);
                double width = w.num(2);
                double height = w.num(3);
                        
                if ((x0 >= 0.0) && (x0 < 10000.0)
                    && (y0 >= 0.0) && (y0 < 10000.0)
                    && (width >= 0.0) && (width < 10000.0)
                    && (height >= 0.0) && (height < 10000.0)) {
                                
                        _x0 = (int) x0;
                        _y0 = (int) y0;
                        _width = (int) width;
                        _height = (int) height;
                        r = 0;

                        r_debug("workspace: x0 %d, y0 %d, "
                                "width %d px, height %d px", 
                                _x0, _y0, _width, _height);
                                
                } else {
                        r_err("ImageCropper: Invalid workspace values");
                }
        
                return r;
        }

        int ImageCropper::set_parameter(const char *name, JSON value)
        {
                int r = -1;
                if (rstreq(name, "workspace")) {
                        r = set_workspace(value);
                } else {
                        r_warn("ImageCropper: unknown parameter: %s", name);
                }
                return r;                
        }

        double ImageCropper::map_meters_to_pixels(double meters)
        {
                printf("x[0]=%f, x[1]=%f\n", _range._x[0], _range._x[1]);
                double width_meters = fabs(_range._x[1] - _range._x[0]);
                double meters_to_pixels = (double) _width / width_meters;
                return meters * meters_to_pixels;
        }

        void ImageCropper::crop(IFileCabinet *session,
                                Image &camera,
                                double tool_diameter,
                                Image &out)
        {
                if (_width > 0 && _height > 0) {

                        double diameter = map_meters_to_pixels(tool_diameter);
                        int border = (int) (diameter / 2.0);
                        int x0 = _x0 - border;
                        int width = _width + 2 * border;
                        int y0 = camera.height() - _y0 - _height - border;
                        int height = _height + 2 * border;

                        session->store("camera", camera);

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
                                Image cropped;
                                camera.crop(x0, y0, width, height, cropped);
                                session->store("cropped_debug", cropped);

                                cropped.scale(3, out);
                                session->store("scaled_debug", out);
                                
                        } else {
                                
                                camera.crop(x0, y0, width, height, out);
                                session->store("cropped_debug", out);
                        }
                        
                } else {
                        r_err("ImageCropper::crop: not initialized");
                }
        }
}
