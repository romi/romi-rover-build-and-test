/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
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
#ifndef __ROMI_SPEED_CONVERTER_H
#define __ROMI_SPEED_CONVERTER_H

#include "JsonCpp.h"

namespace romi {
        
        struct WheelSpeeds
        {
                double left;
                double right;
        };
        
        struct SpeedConverter
        {
                bool use_speed_curve;
                bool use_direction_curve;
                double speed_curve_exponent;     // range (0.0, 10.0]
                double direction_curve_exponent; // range (0.0, 10.0]
                double speed_multiplier;         // range [0.1, 1.0]
                double direction_multiplier;     // range [0.1, 1.0]

                SpeedConverter() {
                        // By default, very safe values
                        use_speed_curve = false;
                        use_direction_curve = false;
                        speed_curve_exponent = 0.001;
                        direction_curve_exponent = 0.001;
                        speed_multiplier = 0.1;
                        direction_multiplier = 0.1;
                }
                
                void parse(JsonCpp& config);
                bool is_valid();
                double map_exponential(double x, double alpha);
                double apply_speed_curve(double speed);
                double apply_direction_curve(double direction);
                WheelSpeeds compute_speeds(double speed, double direction);
        };
}

#endif // __ROMI_SPEED_CONVERTER_H
