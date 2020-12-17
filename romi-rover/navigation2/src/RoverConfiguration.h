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
#ifndef __ROMI_ROVER_CONFIGURATION_H
#define __ROMI_ROVER_CONFIGURATION_H

#include <math.h>
#include <JsonCpp.h>

namespace romi {
        
        class RoverConfiguration
        {
        public:
                double encoder_steps;
                double wheel_diameter;
                double maximum_speed;
                double wheel_base;
                double wheel_circumference;
                double max_revolutions_per_sec;

                RoverConfiguration(JsonCpp &config) {
                        encoder_steps = config.num("encoder_steps");
                        wheel_diameter = config.num("wheel_diameter");
                        maximum_speed = config.num("maximum_speed");
                        wheel_base = config.num("wheel_base");
                        wheel_circumference = M_PI * wheel_diameter;
                        max_revolutions_per_sec = maximum_speed / wheel_circumference;
                }
        };
}

#endif // __ROMI_ROVER_CONFIGURATION_H
