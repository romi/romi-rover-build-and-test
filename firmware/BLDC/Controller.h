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

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include "ITimerHandler.h"
#include "BLDC.h"

class Controller : public ITimerHandler
{
protected:
        BLDC& motor_;
        float speed_;
        float target_speed_;
        float max_acceleration_;
        float last_time_;
        
public:
        Controller(BLDC& motor, float max_acceleration);
        virtual ~Controller() override = default;

        void update(float time) override;
        void set_target_speed(float v);
};

#endif // __CONTROLLER_H
