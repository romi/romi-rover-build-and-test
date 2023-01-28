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
#include "PowerController.h"

PowerController::PowerController(IEncoder& encoder, IBLDC& motor)
        : encoder_(encoder),
          motor_(motor),
          target_speed_(0.0),
          last_angle_(0.0),
          last_time_(0.0),
          speeds_index_(0),
          last_error_(0.0f),
          error_index_(0),
          power_(0.0f),
          delta_power_(0.005f)
{
        for (int i = 0; i < kLengthAvgBuffer; i++) {
                speeds_[i] = 0.0f;
        }
        for (int i = 0; i < kLengthErrorBuffer; i++) {
                error_[i] = 0.0f;
        }
}

void PowerController::update(float time)
{
        update_speed(time);
}

static int counter = 0;

float PowerController::get_instantaneous_speed(float dt)
{
        float angle = encoder_.get_angle();
        float diff = angle - last_angle_;
        if (diff > 0.7f)
                diff = diff - 1.0f;
        else if (diff < -0.7f)
                diff = diff + 1.0f;
        float speed = diff / dt;

        if (0 /*&& speeds_index_ == 0*/) {
                Serial.print("Angle=");
                Serial.print(angle, 5);
                Serial.print(", Last=");
                Serial.print(last_angle_, 5);
                Serial.print(", Diff=");
                Serial.print(diff, 5);
                Serial.print(", dt=");
                Serial.print(dt, 7);
                Serial.print(", V=");
                Serial.println(speed, 5);
                counter = 0;
        }

        last_angle_ = angle;
        return speed;
}

float PowerController::estimate_speed(float dt)
{
        float current_speed = get_instantaneous_speed(dt);
        
        speeds_[speeds_index_++] = current_speed;
        if (speeds_index_ == kLengthAvgBuffer)
                speeds_index_ = 0;
        
        float speed = 0.0f;
        for (int i = 0; i < kLengthAvgBuffer; i++) {
                speed += speeds_[i];
        }
        speed /= kLengthAvgBuffer;

        if (0 /*&& speeds_index_ == 0*/) {
                Serial.print("Vt=");
                Serial.print(current_speed, 5);
                Serial.print(", Vm=");
                Serial.print(speed, 5);
                Serial.print(", Ev=");
                Serial.println(speed-target_speed_, 5);
                counter = 0;
        }
        
        return speed;
}

void PowerController::update_speed(float time)
{
        // if (time - last_time_ > 0.020f) {
        //         float dt = time - last_time_;
        //         float speed = estimate_speed(dt);

        //         float ep = target_speed_ - speed;
        //         if (target_speed_ < 0.0f) {
        //                 ep = -ep;
        //         }
                
        //         if (ep > 0.0f) {
        //                 power_ += delta_power_;
        //         } else {
        //                 power_ -= delta_power_;
        //         }

        //         // error_[error_index_++] = ep;
        //         // if (error_index_ == kLengthErrorBuffer)
        //         //         error_index_ = 0;
                
        //         // float ei = 0.0f;
        //         // for (int i = 0; i < kLengthErrorBuffer; i++) {
        //         //         ei += error_[i];
        //         // }
        //         // ei /= (float) kLengthErrorBuffer;

        //         // float ed = ep - last_error_;

        //         // float kp = 1.5f;
        //         // float ki = 2.0f;
        //         // float kd = 0.5f;
                
        //         // power_ = kp * ep + ki * ei + kd * ed;
                
        //         if (power_ > 1.0f)
        //                 power_ = 1.0f;
        //         else if (power_ < 0.0f)
        //                 power_ = 0.0f;
                
        //         Serial.print("ep=");
        //         Serial.print(ep, 5);
        //         // Serial.print(", ei=");
        //         // Serial.print(ki * ei, 5);
        //         // Serial.print(", ed=");
        //         // Serial.print(kd * ed, 5);
        //         Serial.print(" P=");
        //         Serial.println(power_, 5);

        //         motor_.set_power(power_);
                
        //         last_error_ = ep;
        //         last_time_ = time;
        // }
}

void PowerController::set_target_speed(float v)
{
        target_speed_ = v;
}

void PowerController::init_start_speed()
{
        for (int i = 0; i < 10; i++) {
                last_angle_ = encoder_.get_angle();
                delay(100);
        }        
}
