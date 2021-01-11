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
#ifndef __OQUAM_STEPPER_SETTINGS_H_
#define __OQUAM_STEPPER_SETTINGS_H

#include <JsonCpp.h>

namespace romi {
                
        class StepperSettings
        {
        protected:
                
                void parse_steps_per_revolution(JsonCpp& json);
                void parse_microsteps(JsonCpp& json);
                void parse_gears_ratio(JsonCpp& json);
                void parse_maximum_rpm(JsonCpp& json);
                void parse_displacement_per_revolution(JsonCpp& json);
                void parse_maximum_acceleration(JsonCpp& json);
                void parse_array(JsonCpp& array, double *values);
                void compute_maximum_speed();
                void compute_steps_per_meter();

        public:

                double steps_per_revolution[3];
                double microsteps[3];
                double gears_ratio[3];
                double maximum_rpm[3];
                double displacement_per_revolution[3]; // in meter
                double maximum_acceleration[3];
                double maximum_speed[3];
                double steps_per_meter[3];
                
                StepperSettings(JsonCpp& json);
                virtual ~StepperSettings() = default;

        };
}

#endif // __OQUAM_STEPPER_SETTINGS_H
