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

#ifndef __POWERCONTROLLER_H
#define __POWERCONTROLLER_H

#include "ITimerHandler.h"
#include "ISpeedController.h"
#include "IBLDC.h"
#include "IEncoder.h"

class PowerController : public ITimerHandler, public ISpeedController
{
protected:
        static const int kLengthAvgBuffer = 8;
        static const int kLengthErrorBuffer = 20;
        IEncoder& encoder_;
        IBLDC& motor_;
        float target_speed_;
        float last_angle_;
        float last_time_;
        float speeds_[kLengthAvgBuffer];
        int speeds_index_;
        float last_error_;
        float error_[kLengthErrorBuffer];
        int error_index_;
        float power_;
        float delta_power_;
        
        float get_instantaneous_speed(float dt);
        float estimate_speed(float dt);

public:
        PowerController(IEncoder& encoder, IBLDC& motor);
        virtual ~PowerController() override = default;

        void update(float time) override;
        void update_speed(float time) override;
        void set_target_speed(float v) override;
        void init_start_speed() override;
};

#endif // __POWERCONTROLLER_H
