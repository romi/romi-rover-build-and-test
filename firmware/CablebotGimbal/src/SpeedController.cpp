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

#include "SpeedController.h"

SpeedController::SpeedController(IBLDC& motor, ISpeedController& power_controller,
                                 float max_acceleration)
        : motor_(motor),
          power_controller_(power_controller),
          speed_(0.0),
          target_speed_(0.0),
          max_acceleration_(max_acceleration),
          last_time_(0.0)
{
}

void SpeedController::update(float time)
{
        update_speed(time);
}

void SpeedController::update_speed(float time)
{
        float dt = time - last_time_;
        if (speed_ < target_speed_) {
                speed_ += max_acceleration_ * dt;
                if (speed_ > target_speed_)
                        speed_ = target_speed_;
        } else if (speed_ > target_speed_) {
                speed_ -= max_acceleration_ * dt;
                if (speed_ < target_speed_)
                        speed_ = target_speed_;
        }
        
        power_controller_.set_target_speed(speed_);
        power_controller_.update_speed(time);
        
        float delta = speed_ * dt;
        motor_.incr_position(delta);
        last_time_ = time;
}

void SpeedController::set_target_speed(float v)
{
        target_speed_ = v;
}

void SpeedController::init_start_speed()
{
}
