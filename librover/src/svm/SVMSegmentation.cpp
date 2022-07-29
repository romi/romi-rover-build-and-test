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

#include <Logger.h>
#include <stdexcept>
#include "svm/SVMSegmentation.h"

namespace romi {
        
        SVMSegmentation::SVMSegmentation(nlohmann::json& params) : _b(0.0)
        {
                try {
                        set_parameter_a(params["a"]);
                        set_parameter_b(params["b"]);
                } catch (nlohmann::json::exception& je) {
                        r_err("SVMSegmentation: Failed to parse the parameters: %s",
                              je.what());
                        throw std::runtime_error("SVMSegmentation: bad config");
                }
        }
        
        SVMSegmentation::SVMSegmentation(float a[3], float b) : _b(0.0)
        {
                set_coefficients(a);
                set_intercept(b);
        }

        void SVMSegmentation::set_coefficients(float a[3])
        {
                for (int i = 0; i < 3; i++)
                        _a[i] = a[i];
        }

        float *SVMSegmentation::get_coefficients()
        {
                return _a;
        }
                
        void SVMSegmentation::set_intercept(float b)
        {
                _b = b;
        }
                
        float SVMSegmentation::get_intercept()
        {
                return _b;
        }

        void SVMSegmentation::set_parameter_a(nlohmann::json value)
        {
                float a[3];
                a[0] = (float) value[0];
                a[1] = (float) value[1];
                a[2] = (float) value[2];
                set_coefficients(a);
        }
        
        void SVMSegmentation::set_parameter_b(nlohmann::json value)
        {
                set_intercept((float) value);
        }

        bool SVMSegmentation::create_mask(ISession &session, Image &image, Image &mask)
        {
                (void) session;
                
                if (image.type() != Image::RGB) {
                        r_err("SVMSegmentation::create_mask: Expected an RGB input image");
                        return false;
                }
                
                mask.init(Image::BW, image.width(), image.height());
                
                size_t len = image.width() * image.height();
                auto& a = image.data();
                auto& r = mask.data();
                
                for (size_t i = 0, j = 0; i < len; i++, j += 3) {
                        float x = (static_cast<float>(255.0 * a[j] * _a[0]
                                                      + 255.0 * a[j + 1] * _a[1]
                                                      + 255.0 * a[j + 2] * _a[2]
                                                      + _b));
                        r[i] = (x > 0.0f)? 1.0f : 0.0f;
                }
                return true;
        }
}
