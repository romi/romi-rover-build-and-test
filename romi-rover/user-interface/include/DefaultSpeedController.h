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
#ifndef __ROMI_DEFAULT_SPEEDCONTROLLER_H
#define __ROMI_DEFAULT_SPEEDCONTROLLER_H

#include <math.h>
#include "SpeedController.h"
#include "Navigation.h"
#include "JsonCpp.h"
#include "SpeedConverter.h"

namespace romi {

        class DefaultSpeedController : public SpeedController
        {
        protected:
                Navigation &_navigation;
                SpeedConverter _fast;
                SpeedConverter _accurate;
                
                bool send_moveat(double left, double right);
                
        public:
                
                DefaultSpeedController(Navigation &navigation, JsonCpp& config);
                DefaultSpeedController(Navigation &navigation,
                                       SpeedConverter &fast,
                                       SpeedConverter &accurate);
                virtual ~DefaultSpeedController() override = default;
                
                bool stop() override;
                bool drive_at(double speed, double direction) override;
                bool drive_accurately_at(double speed, double direction) override;
                bool spin(double direction) override;
        };
}

#endif // __ROMI_DEFAULT_SPEEDCONTROLLER_H
