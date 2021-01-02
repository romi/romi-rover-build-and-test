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
#include "StepperSettings.h"

namespace romi {
        
        StepperSettings::StepperSettings(JsonCpp& json)
        {
                try {
                        parse_steps_per_revolution(json);
                        parse_microsteps(json);
                        parse_gears_ratio(json);
                        parse_maximum_rpm(json);
                        parse_displacement_per_revolution(json);
                        parse_maximum_acceleration(json);
                        compute_maximum_speed();
                        compute_steps_per_meter();
                        
                } catch (JSONError& je) {
                        r_err("StepperSettings: Failed to parse the JSON settings");
                        throw std::runtime_error("Invalid settings");;
                }
        }
        
        void StepperSettings::parse_steps_per_revolution(JsonCpp& json)
        {
                JsonCpp array = json.array("steps-per-revolution");
                parse_array(array, steps_per_revolution);
        }
        
        void StepperSettings::parse_microsteps(JsonCpp& json)
        {
                JsonCpp array = json.array("microsteps");
                parse_array(array, microsteps);
        }
        
        void StepperSettings::parse_gears_ratio(JsonCpp& json)
        {
                JsonCpp array = json.array("gears-ratio");
                parse_array(array, gears_ratio);
        }
        
        void StepperSettings::parse_maximum_rpm(JsonCpp& json)
        {
                JsonCpp array = json.array("maximum-rpm");
                parse_array(array, maximum_rpm);
        }
        
        void StepperSettings::parse_displacement_per_revolution(JsonCpp& json)
        {
                JsonCpp array = json.array("displacement-per-revolution");
                parse_array(array, displacement_per_revolution);
        }
        
        void StepperSettings::parse_maximum_acceleration(JsonCpp& json)
        {
                JsonCpp array = json.array("maximum-acceleration");
                parse_array(array, maximum_acceleration);
        }
        
        void StepperSettings::parse_array(JsonCpp& array, double *values)
        {
                for (int i = 0; i < 3; i++) {
                        values[i] = array.num(i);
                }
        }

        void StepperSettings::compute_maximum_speed()
        {
                for (int i = 0; i < 3; i++) {
                        maximum_speed[i] = (fabs(displacement_per_revolution[i])
                                            * maximum_rpm[i] / 60.0
                                            / gears_ratio[i]);
                }
        }

        void StepperSettings::compute_steps_per_meter()
        {
                for (int i = 0; i < 3; i++) {
                        steps_per_meter[i] = ((gears_ratio[i]
                                               * microsteps[i]
                                               * steps_per_revolution[i])
                                              / displacement_per_revolution[i]);
                }
        }
}
