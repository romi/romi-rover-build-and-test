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

#ifndef __ROMI_SVM_SEGMENTATION_H
#define __ROMI_SVM_SEGMENTATION_H

#include "../IImageSegmentation.h"

namespace romi {

        class SVMSegmentation : public IImageSegmentation
        {
        protected:
                float _a[3];
                float _b;

                void set_parameter_a(JSON value);
                void set_parameter_b(JSON value);

        public:
                SVMSegmentation() {}
                
                SVMSegmentation(float a[3], float b) {
                        set_coefficients(a);
                        set_intercept(b);
                }
                
                virtual ~SVMSegmentation() override = default;

                void set_coefficients(float a[3]) {
                        for (int i = 0; i < 3; i++)
                                _a[i] = a[i];
                }

                float *get_coefficients() {
                        return _a;
                }
                
                void set_intercept(float b) {
                        _b = b;
                }
                
                float get_intercept() {
                        return _b;
                }

                void set_parameter(const char *name, JSON value) override;
                void segment(IFolder *session, Image &image, Image &mask) override;
        };
}

#endif // __ROMI_SVM_SEGMENTATION_H
