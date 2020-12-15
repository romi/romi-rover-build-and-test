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
#ifndef __ROMI_SPEEDCONTROLLER_H
#define __ROMI_SPEEDCONTROLLER_H

#include <math.h>
#include "ISpeedController.h"
#include "INavigation.h"
#include "JSON.h"

namespace romi {
        
        struct SpeedControllerSettings
        {
                bool use_speed_curve;
                bool use_direction_curve;
                double speed_curve_exponent;     // range (0.0, 10.0]
                double direction_curve_exponent; // range (0.0, 10.0]
                double speed_multiplier;         // range [0.1, 1.0]
                double direction_multiplier;     // range [0.1, 1.0]

                void parse(JSON &config) {
                        use_speed_curve = config.boolean("use-speed-curve");
                        speed_curve_exponent = config.boolean("direction-curve-exponent");
                        use_direction_curve = config.num("use-direction-curve");
                        direction_curve_exponent = config.num("direction-curve-exponent");
                        speed_multiplier = config.num("speed-multiplier");
                        direction_multiplier = config.num("direction-multiplier");
                }

                bool valid() {
                        return (speed_curve_exponent > 0.0
                                && speed_curve_exponent <= 10.0
                                && direction_curve_exponent > 0.0
                                && direction_curve_exponent <= 10.0
                                && speed_multiplier >= 0.1
                                && speed_multiplier <= 1.0
                                && direction_multiplier >= 0.1
                                && direction_multiplier <= 1.0);
                }
        };
        
        class SpeedController : public ISpeedController
        {
        protected:
                INavigation &_navigation;
                SpeedControllerSettings _fast;
                SpeedControllerSettings _accurate;
                
                double map_exponential(double x, double alpha);
                double map_speed(SpeedControllerSettings &settings, double speed);
                double map_direction(SpeedControllerSettings &settings, double direction);
                bool send_moveat(double left, double right);
                bool do_drive_at(SpeedControllerSettings &settings,
                                 double speed, double direction);
                
        public:
                
                SpeedController(INavigation &navigation, JSON &config);
                SpeedController(INavigation &navigation,
                                SpeedControllerSettings &fast,
                                SpeedControllerSettings &accurate);
                virtual ~SpeedController() override = default;
                
                bool stop() override;
                bool drive_at(double speed, double direction) override;
                bool drive_accurately_at(double speed, double direction) override;
                bool spin(double direction) override;
        };
}

#endif // __ROMI_SPEEDCONTROLLER_H
