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

#include "Arduino.h"
#include "PositionController.h"

PositionController::PositionController(IEncoder& encoder,
                                       ISpeedController& speed_controller)
        : encoder_(encoder),
          speed_controller_(speed_controller),
          start_position_(0.0f),
          target_position_(0.0),
          last_time_(0.0)
{
}

void PositionController::init_start_position()
{
        for (int i = 0; i < 10; i++) {
                start_position_ = encoder_.get_angle();
                delay(100);
                // Serial.print("S=");
                // Serial.println(start_position_, 5);
        }
}

void PositionController::update(float time)
{
        update_position(time);
}

static int counter = 0;

void PositionController::update_position(float time)
{
        float position = encoder_.get_angle() - start_position_; // TODO: get_position???
        if (position < 0.0f)
                position += 1.0f;
                
        float diff = position - target_position_;
        if (diff > 0.5f) {
                diff = diff - 1.0f;
        }
        
        float speed = 0.0f;
        if (diff < -0.01f) {
                speed = 0.1f;
        } else if (diff < -0.005f) {
                speed = 0.05f;
        } else if (diff > 0.01f) {
                speed = -0.1f;                
        } else if (diff > 0.005f) {
                speed = -0.05f;                
        }

        if (0 && ++counter == 50) {
                Serial.print("T=");
                Serial.print(target_position_, 5);
                Serial.print(", P=");
                Serial.print(position, 5);
                Serial.print(", D=");
                Serial.print(diff, 5);
                Serial.print(", V=");
                Serial.println(speed);
                counter = 0;
        }
        
        speed_controller_.set_target_speed(speed);
        speed_controller_.update_speed(time);
        
        last_time_ = time;
}

void PositionController::set_target_position(float position)
{
        target_position_ = position;
}

