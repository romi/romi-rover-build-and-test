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

#include <r.h>
#include "SpeedController.h"

namespace romi {
        
        SpeedController::SpeedController(INavigation &navigation)
                : _navigation(navigation)
        {
                _map_speed_exponential = true;
                _map_direction_exponential = true;
                        
                _alpha_speed = 3.0;
                _alpha_direction = 3.0;
                        
                _speed_coeff = 1.0;
                _direction_coeff = 0.4;
                        
                _speed_coeff_accurate = 0.3;
                _direction_coeff_accurate = 0.15;
        }
                
        double SpeedController::map_exponential(double x, double alpha)
        {
                return ((exp(alpha * x) - 1.0) / (exp(alpha) - 1.0));
        }
                
        double SpeedController::map_speed(double speed)
        {
                double retval = speed;
                if (_map_speed_exponential) {
                        double sign = (speed >= 0.0)? 1.0 : -1.0;
                        double x = fabs(speed);
                        double y = map_exponential(x, _alpha_speed);
                        retval = sign * y;
                }
                return retval;
        }

        double SpeedController::map_direction(double direction)
        {
                double retval = direction;
                if (_map_direction_exponential) {
                        double sign = (direction >= 0.0)? 1.0 : -1.0;
                        double x = fabs(direction);
                        double y = map_exponential(x, _alpha_direction);
                        retval = sign * y;
                }
                return retval;
        }

        void SpeedController::send_moveat(double left, double right)
        {
                if (left < -1.0)
                        left = -1.0;
                else if (left > 1.0)
                        left = 1.0;
                if (right < -1.0)
                        right = -1.0;
                else if (right > 1.0)
                        right = 1.0;
                _navigation.moveat(left, right);
        }
                
        void SpeedController::stop()
        {
                _navigation.moveat(0.0, 0.0);
        }
                
        void SpeedController::drive_at(double speed, double direction)
        {
                r_debug("SpeedController::drive_at(speed=%0.3f, direction=%0.3f)",
                        speed, direction);
                double v = map_speed(speed);
                double d = map_direction(direction);
                double left = (_speed_coeff * v
                               + _direction_coeff * d);
                double right = (_speed_coeff * v
                                - _direction_coeff * d);
                send_moveat(left, right);
        }
                
        void SpeedController::drive_accurately_at(double speed, double direction)
        {
                r_debug("SpeedController::drive_accurately_at"
                        "(speed=%0.3f, direction=%0.3f)",
                        speed, direction);
                double v = map_speed(speed);
                double d = map_direction(direction);
                double left = (_speed_coeff_accurate * v
                               + _direction_coeff_accurate * d);
                double right = (_speed_coeff_accurate * v
                                - _direction_coeff_accurate * d);
                send_moveat(left, right);
        }

        void SpeedController::spin(double direction)
        {
                r_debug("SpeedController::spin(direction=%0.3f)", direction);
                double d = map_direction(direction);
                double left = _direction_coeff * d;
                double right = -left;
                send_moveat(left, right);
        }        
}
