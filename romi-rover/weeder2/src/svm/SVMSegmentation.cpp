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
#include "SVMSegmentation.h"

namespace romi {

        void SVMSegmentation::set_parameter_a(JsonCpp value)
        {
                float a[3];
                a[0] = (float) value.num(0);
                a[1] = (float) value.num(1);
                a[2] = (float) value.num(2);
                set_coefficients(a);
        }
        
        void SVMSegmentation::set_parameter_b(JsonCpp value)
        {
                set_intercept((float) value.num());
        }

        void SVMSegmentation::set_parameter(const char *name, JsonCpp value)
        {
                if (rstreq(name, "a")) {
                        set_parameter_a(value);
                } else if (rstreq(name, "b")) {
                        set_parameter_b(value);
                } else {
                        r_warn("SVMSegmentation: unknown parameter: %s", name);
                }
        }

        void SVMSegmentation::segment(IFolder &session, Image &image, Image &mask) 
        {
                if (image.type() != IMAGE_RGB)
                        throw std::runtime_error("SVMSegmentation::segment: "
                                                 "Expected an RGB input image");

                image_t *out = new_image(IMAGE_BW, image.width(), image.height());
                int len = image.width() * image.height();
                float *a = image.data();
                
                for (int i = 0, j = 0; i < len; i++, j += 3) {
                        float x = (255.0 * a[j] * _a[0]
                                   + 255.0 * a[j+1] * _a[1]
                                   + 255.0 * a[j+2] * _a[2]
                                   + _b);
                        out->data[i] = (x > 0.0f)? 1.0f : 0.0f;
                }

                mask.moveto(out);
        }
}
