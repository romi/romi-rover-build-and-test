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
#include "DefaultSpeedController.h"
#include <stdexcept>

namespace romi {
        
        DefaultSpeedController::DefaultSpeedController(Navigation &navigation,
                                                       JsonCpp& config)
                : _navigation(navigation)
        {
                try {
                        JsonCpp fast_config = (config.get("user-interface")
                                               .get("speed-controller")
                                               .get("fast"));
                        JsonCpp accurate_config = (config.get("user-interface")
                                                   .get("speed-controller")
                                                   .get("accurate"));
                
                        _fast.parse(fast_config);
                        _accurate.parse(accurate_config);
                        
                } catch (JSONError &je) {
                        r_err("DefaultSpeedController::DefaultSpeedController: failed to "
                              "parse the configuration");
                        throw je;
                }
                
                if (!_accurate.is_valid()) 
                        throw std::range_error("Invalid settings for "
                                               "accurate speed controller");
                if (!_fast.is_valid())
                        throw std::range_error("Invalid settings for "
                                               "fast speed controller");
        }

        DefaultSpeedController::DefaultSpeedController(Navigation &navigation,
                                                       SpeedConverter &fast,
                                                       SpeedConverter &accurate)
                : _navigation(navigation)
        {
                _fast = fast;
                if (!_fast.is_valid()) 
                        throw std::range_error("Invalid settings for "
                                               "fast speed controller");
                
                _accurate = accurate;
                if (!_accurate.is_valid()) 
                        throw std::range_error("Invalid settings for "
                                               "accurate speed controller");
        }

        bool DefaultSpeedController::send_moveat(double left, double right)
        {
                // r_debug("SpeedController::moveat(left=%.3f,right=%.3f)", left, right); 
                left = std::clamp(left, -1.0, 1.0); 
                right = std::clamp(right, -1.0, 1.0);
                return _navigation.moveat(left, right);
        }
                
        bool DefaultSpeedController::stop()
        {
                return _navigation.stop();
        }
        
        bool DefaultSpeedController::drive_at(double speed, double direction)
        {
                WheelSpeeds speeds = _fast.compute_speeds(speed, direction);
                return send_moveat(speeds.left, speeds.right);
        }
                
        bool DefaultSpeedController::drive_accurately_at(double speed, double direction)
        {
                WheelSpeeds speeds = _accurate.compute_speeds(speed, direction);
                return send_moveat(speeds.left, speeds.right);
        }

        bool DefaultSpeedController::spin(double direction)
        {
                WheelSpeeds speeds = _accurate.compute_speeds(0.0, direction);
                return send_moveat(speeds.left, speeds.right);
        }        
}
