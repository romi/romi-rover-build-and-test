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
#include "SpeedConverter.h"
#include <math.h>
#include <algorithm>

namespace romi {
        
        void SpeedConverter::parse(JsonCpp& config) {
                use_speed_curve = config.boolean("use-speed-curve");
                speed_curve_exponent = config.num("speed-curve-exponent");
                use_direction_curve = config.boolean("use-direction-curve");
                direction_curve_exponent = config.num("direction-curve-exponent");
                speed_multiplier = config.num("speed-multiplier");
                direction_multiplier = config.num("direction-multiplier");
        }

        bool SpeedConverter::valid()
        {
                return (speed_curve_exponent > 0.0
                        && speed_curve_exponent <= 10.0
                        && direction_curve_exponent > 0.0
                        && direction_curve_exponent <= 10.0
                        && speed_multiplier >= 0.1
                        && speed_multiplier <= 1.0
                        && direction_multiplier >= 0.1
                        && direction_multiplier <= 1.0);
        }

        double SpeedConverter::map_exponential(double x, double alpha)
        {
                return ((exp(alpha * x) - 1.0) / (exp(alpha) - 1.0));
        }

        double SpeedConverter::apply_speed_curve(double speed)
        {
                double retval = speed;
                if (use_speed_curve) {
                        double sign = (speed >= 0.0)? 1.0 : -1.0;
                        double x = fabs(speed);
                        double y = map_exponential(x, speed_curve_exponent);
                        retval = sign * y;
                }
                return retval;
        }

        double SpeedConverter::apply_direction_curve(double direction)
        {
                double retval = direction;
                if (use_direction_curve) {
                        double sign = (direction >= 0.0)? 1.0 : -1.0;
                        double x = fabs(direction);
                        double y = map_exponential(x, direction_curve_exponent);
                        retval = sign * y;
                }
                return retval;
        }

        WheelSpeeds SpeedConverter::compute_speeds(double speed, double direction)
        {
                WheelSpeeds speeds;
                        
                speed = std::clamp(speed, -1.0, 1.0); 
                direction = std::clamp(direction, -1.0, 1.0);
                
                speed = apply_speed_curve(speed);
                direction = apply_direction_curve(direction);
                        
                speeds.left = (speed_multiplier * speed
                               + direction_multiplier * direction);
                        
                speeds.right = (speed_multiplier * speed
                                - direction_multiplier * direction);

                return speeds;
        }

}
