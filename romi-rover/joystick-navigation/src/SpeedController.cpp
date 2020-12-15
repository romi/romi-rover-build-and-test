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
#include <algorithm>
#include "SpeedController.h"
#include <stdexcept>

namespace romi {
        
        SpeedController::SpeedController(INavigation &navigation, JSON &config)
                : _navigation(navigation)
        {
                JSON fast_config = config.get("speed-controller").get("fast");
                JSON accurate_config = config.get("speed-controller").get("accurate");
                
                _fast.parse(fast_config);
                if (!_fast.valid()) 
                        throw std::range_error("Invalid settings for "
                                               "fast speed controller");
                
                _accurate.parse(accurate_config);
                if (!_accurate.valid()) 
                        throw std::range_error("Invalid settings for "
                                               "accurate speed controller");                
        }

        SpeedController::SpeedController(INavigation &navigation,
                                         SpeedControllerSettings &fast,
                                         SpeedControllerSettings &accurate)
                : _navigation(navigation)
        {
                _fast = fast;
                if (!_fast.valid()) 
                        throw std::range_error("Invalid settings for "
                                               "fast speed controller");
                
                _accurate = accurate;
                if (!_accurate.valid()) 
                        throw std::range_error("Invalid settings for "
                                               "accurate speed controller");
        }

        double SpeedController::map_exponential(double x, double alpha)
        {
                return ((exp(alpha * x) - 1.0) / (exp(alpha) - 1.0));
        }
                
        double SpeedController::map_speed(SpeedControllerSettings &settings,
                                          double speed)
        {
                double retval = speed;
                if (settings.use_speed_curve) {
                        double sign = (speed >= 0.0)? 1.0 : -1.0;
                        double x = fabs(speed);
                        double y = map_exponential(x, settings.speed_curve_exponent);
                        retval = sign * y;
                }
                return retval;
        }

        double SpeedController::map_direction(SpeedControllerSettings &settings,
                                              double direction)
        {
                double retval = direction;
                if (settings.use_direction_curve) {
                        double sign = (direction >= 0.0)? 1.0 : -1.0;
                        double x = fabs(direction);
                        double y = map_exponential(x, settings.direction_curve_exponent);
                        retval = sign * y;
                }
                return retval;
        }

        bool SpeedController::send_moveat(double left, double right)
        {
                // r_debug("SpeedController::moveat(left=%.3f,right=%.3f)", left, right); 
                left = std::clamp(left, -1.0, 1.0); 
                right = std::clamp(right, -1.0, 1.0);
                return _navigation.moveat(left, right);
        }
                
        bool SpeedController::stop()
        {
                return _navigation.stop();
        }
        
        bool SpeedController::do_drive_at(SpeedControllerSettings &settings,
                                          double speed, double direction)
        {
                // r_debug("SpeedController::do_drive_at(speed=%0.3f, direction=%0.3f)",
                //         speed, direction);
                
                speed = std::clamp(speed, -1.0, 1.0); 
                direction = std::clamp(direction, -1.0, 1.0);
                
                speed = map_speed(settings, speed);
                direction = map_direction(settings, direction);
                
                double left = (settings.speed_multiplier * speed
                               + settings.direction_multiplier * direction);
                
                double right = (settings.speed_multiplier * speed
                                - settings.direction_multiplier * direction);
                
                return send_moveat(left, right);
        }
                
        bool SpeedController::drive_at(double speed, double direction)
        {
                return do_drive_at(_fast, speed, direction);
        }
                
        bool SpeedController::drive_accurately_at(double speed, double direction)
        {
                return do_drive_at(_accurate, speed, direction);
        }

        bool SpeedController::spin(double direction)
        {
                return do_drive_at(_accurate, 0.0, direction);
        }        
}
