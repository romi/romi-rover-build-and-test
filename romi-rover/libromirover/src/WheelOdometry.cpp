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
#include "WheelOdometry.h"

namespace romi {
        
        WheelOdometry::WheelOdometry(RoverConfiguration &rover_config,
                                     double left_encoder,
                                     double right_encoder,
                                     double timestamp)
        {
                wheel_circumference = M_PI * rover_config.wheel_diameter;
                wheel_base = rover_config.wheel_base;
                encoder_steps = rover_config.encoder_steps;
                encoder[0] = left_encoder;
                encoder[1] = right_encoder;
                for (int i = 0; i < 2; i++) {
                        instantaneous_speed[i] = 0;
                        filtered_speed[i] = 0;
                }
                last_timestamp = timestamp;
        }
        
        WheelOdometry::~WheelOdometry()
        {
        }

        void WheelOdometry::get_location(double &x, double &y)
        {
                SynchronizedCodeBlock sync(_m);
                x = displacement[0];
                y = displacement[1];
        }

        void WheelOdometry::get_speed(double &vx, double &vy)
        {
                SynchronizedCodeBlock sync(_m);
                vx = filtered_speed[0];
                vy = filtered_speed[1];
        }
                
        double WheelOdometry::get_orientation()
        {
                SynchronizedCodeBlock sync(_m);
                return theta;
        }

        void WheelOdometry::set_encoders(double left, double right, double timestamp)
        {
                double dx, dy;
                double dL, dR;
                double half_wheel_base = 0.5 * wheel_base;
                double alpha;

                SynchronizedCodeBlock sync(_m);
                
                // r_debug("encL %f, encR %f steps", left, right);
        
                // dL and dR are the distances travelled by the left and right
                // wheel.
                dL = left - encoder[0];
                dR = right - encoder[1];
                r_debug("dL %f, dR %f steps", dL, dR);
        
                dL = wheel_circumference * dL / encoder_steps;
                dR = wheel_circumference * dR / encoder_steps;
                // r_debug("dL %f, dR %f m", dL, dR);

                // dx and dy are the changes in the location of the rover, in
                // the frame of reference of the rover.
                if (dL == dR) {
                        dx = dL;
                        dy = 0.0;
                        alpha = 0.0;
                } else {
                        double radius = 0.5 * wheel_base * (dL + dR) / (dR - dL);
                        /* r_debug("radius %f", radius); */
                        if (radius >= 0) {
                                alpha = dR / (radius + half_wheel_base);
                        } else {
                                alpha = -dL / (-radius + half_wheel_base);
                        }
                        dx = radius * sin(alpha);
                        dy = radius - radius * cos(alpha);
                }

                // r_debug("dx %f, dy %f, alpha %f", dx, dy, alpha);

                // Convert dx and dy to the changes in the last frame of
                // reference (i.e. relative to the current orientation).
                double c = cos(theta);
                double s = sin(theta);
                double dx_ = c * dx - s * dy;
                double dy_ = s * dx + c * dy;

                // r_debug("dx_ %f, dy_ %f", dx_, dy_);

                displacement[0] += dx_;
                displacement[1] += dy_;
                theta += alpha;
                encoder[0] = left;
                encoder[1] = right;

                double dt = timestamp - last_timestamp;
                if (dt != 0.0) {
                        instantaneous_speed[0] = dx_ / dt;
                        instantaneous_speed[1] = dy_ / dt;
                        
                        filtered_speed[0] = (0.8 * filtered_speed[0]
                                             + 0.2 * instantaneous_speed[0]);
                        filtered_speed[1] = (0.8 * filtered_speed[1]
                                             + 0.2 * instantaneous_speed[1]);
                }
                last_timestamp = timestamp;
        
                // r_debug("displacement:  %f %f - angle %f",
                //         displacement[0], displacement[1], theta * 180.0 / M_PI);
                //r_debug("speed:  %f %f", speed[0], speed[1]);
        }
}
