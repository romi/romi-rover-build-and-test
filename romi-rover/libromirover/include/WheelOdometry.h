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

#include <mutex>
#include "NavigationSettings.h"


namespace romi {
              
        using SynchronizedCodeBlock = std::lock_guard<std::mutex>;
        
        class WheelOdometry
        {
        protected:
                std::mutex _m;
                        
                // The current location and orientation
                double instantaneous_speed[2];
                double filtered_speed[2];
                double encoder[2];
                int initialized;
                double last_timestamp;
        
                // The displacement, in meters, and the change in orientation
                // (only tracking the change in yaw) relative to the 'current'
                // location;
                double displacement[2];
                double theta;
                
                double wheel_circumference; 
                double wheel_base;
                double encoder_steps;
                
        public:
                WheelOdometry(NavigationSettings &rover_config,
                              double left_encoder,
                              double right_encoder,
                              double timestamp);
                
                virtual ~WheelOdometry();

                void set_encoders(double left, double right, double timestamp);
                void get_location(double &x, double &y);
                void get_speed(double &vx, double &vy);
                double get_orientation();
        };
}

#endif // __ROMI_WHEEL_ODOMETRY_H
