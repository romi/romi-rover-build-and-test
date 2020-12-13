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
#ifndef __ROMI_WHEEL_ODOMETRY_H
#define __ROMI_WHEEL_ODOMETRY_H

#include "RoverConfiguration.h"
#include "SynchronizedCodeBlock.h"
#include "rover.h"

namespace romi {
        
        class WheelOdometry
        {
        protected:
                mutex_t *_mutex;
                rover_t *_rover;
                        
        public:
                WheelOdometry(RoverConfiguration &rover_config,
                              double left_encoder,
                              double right_encoder,
                              double timestamp) {
                        _mutex = new_mutex();
                        _rover = new_rover(rover_config.wheel_diameter, 
                                           rover_config.wheel_base,
                                           rover_config.encoder_steps);
                        rover_init_encoders(_rover, left_encoder, right_encoder, timestamp);
                }
                
                virtual ~WheelOdometry() {
                        if (_rover)
                                delete_rover(_rover);

                        if (_mutex)
                                delete_mutex(_mutex);
                }

                void get_location(double &x, double &y) {
                        SynchronizedCodeBlock sync(_mutex);
                        vector_t location = rover_get_location(_rover);
                        x = location.x;
                        y = location.y;
                }
                
                double get_orientation() {
                        SynchronizedCodeBlock sync(_mutex);
                        return rover_get_orientation_theta(_rover);
                }

                void set_encoders(double left, double right, double timestamp) {
                        SynchronizedCodeBlock sync(_mutex);
                        rover_set_encoders(_rover, left, right, timestamp);
                }
        };
}

#endif // __ROMI_WHEEL_ODOMETRY_H
