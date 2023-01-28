/*
  bldc_featherwing

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Timothée Wintz, Peter Hanappe

  bldc_featherwing is Arduino firmware to control a brushless motor.

  bldc_featherwing is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#ifndef __SPEEDCONTROLLER_H
#define __SPEEDCONTROLLER_H

#include "ITimerHandler.h"
#include "ISpeedController.h"
#include "IBLDC.h"

class SpeedController : public ITimerHandler, public ISpeedController
{
protected:
        IBLDC& motor_;
        ISpeedController& power_controller_;
        float speed_;
        float target_speed_;
        float max_acceleration_;
        float last_time_;
        
public:
        SpeedController(IBLDC& motor, ISpeedController& power_controller,
                        float max_acceleration);
        virtual ~SpeedController() override = default;

        void update(float time) override;
        void update_speed(float time) override;
        void set_target_speed(float v) override;
        void init_start_speed() override;
};

#endif // __SPEEDCONTROLLER_H
