/*
  bldc_featherwing

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

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
#include "BLDC.h"
#include "IMU.h"
#include <math.h>

float normalize_angle(float angle)
{
        while (angle < 0.0f)
                angle += 1.0f;
        while (angle >= 1.0f)
                angle -= 1.0f;
        return angle;
}

BLDC::BLDC(IPwmGenerator *generator,
           IOutputPin *sleep_pin,
           IOutputPin *reset_pin,
           uint8_t poles)
        
        : generator_(generator),
          sleep_pin_(sleep_pin),
          reset_pin_(reset_pin),
          poles_((float)poles)
{
        set_power(0.0f);
        phase_absolute_ = 0.0f;
        phase_relative_ = 0.0f;
        reset_pin_->set(1.0f);
        sleep();
}

void BLDC::set_power(float p)
{
        if (p < 0.0f)
                p = 0.0f;
        else if (p > 1.0f)
                p = 1.0f;
        
        power_ = p;
        generator_->set_amplitude(p);
}

float BLDC::get_power() const
{
        return power_;
}

void BLDC::wakeup()
{
        sleep_pin_->set(1.0f);
        generator_->enable();
}

void BLDC::sleep()
{
        sleep_pin_->set(0.0f);
        generator_->disable();
}

void BLDC::reset()
{
        reset_pin_->set(0.0f);
        delay(150);
        reset_pin_->set(1.0f);
}

void BLDC::set_phase(float value)
{
        // phase_ = normalizeAngle(value);
        // generator_->set_phase(phase_);
}
        
// void BLDC::incr_phase(float delta)
// {
//         set_phase(phase_ + delta);
// }
        
void BLDC::incr_position(float delta)
{
        phase_absolute_ += delta * poles_;
        phase_relative_ += delta * poles_;
        phase_relative_ = normalize_angle(phase_relative_);
        generator_->set_phase(phase_relative_);
}

// float BLDC::get_position()
// {
//         return phase_absolute_ / poles_;        
// }
