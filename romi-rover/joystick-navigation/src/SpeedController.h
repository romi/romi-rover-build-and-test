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

namespace romi {

        class SpeedController : public ISpeedController
        {
        protected:
                INavigation &_navigation;
                bool _map_speed_exponential;
                bool _map_direction_exponential;
                double _alpha_speed;
                double _alpha_direction;
                double _speed_coeff;
                double _direction_coeff;
                double _speed_coeff_accurate;
                double _direction_coeff_accurate;
                
                double map_exponential(double x, double alpha);
                double map_speed(double speed);
                double map_direction(double direction);
                void send_moveat(double left, double right);
                
        public:
                
                SpeedController(INavigation &navigation);
                virtual ~SpeedController() override = default;
                
                void stop() override;
                void drive_at(double speed, double direction) override;
                void drive_accurately_at(double speed, double direction) override;
                void spin(double direction) override;
        };
}

#endif // __ROMI_SPEEDCONTROLLER_H
