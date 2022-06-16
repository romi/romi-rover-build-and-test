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

#include "session/ISession.h"
#include "weeder/IImageSegmentation.h"

namespace romi {

        class SVMSegmentation : public IImageSegmentation
        {
        protected:
                float _a[3];
                float _b;

                void set_parameter_a(nlohmann::json value);
                void set_parameter_b(nlohmann::json value);

        public:
                explicit SVMSegmentation(nlohmann::json& params);
                SVMSegmentation(float a[3], float b);
                
                virtual ~SVMSegmentation() override = default;

                void set_coefficients(float a[3]);
                float *get_coefficients();
                void set_intercept(float b);
                float get_intercept();

                bool create_mask(ISession &session, Image &image, Image &mask) override;
        };
}

#endif // __ROMI_SVM_SEGMENTATION_H
