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

#ifndef __PWM_GENERATOR_H
#define __PWM_GENERATOR_H

#include "IOutputPin.h"
#include "IPwmGenerator.h"

class PwmGenerator : public IPwmGenerator
{
protected:
        IOutputPin *pwm1;
        IOutputPin *pwm2;
        IOutputPin *pwm3;
        IOutputPin *enable1;
        IOutputPin *enable2;
        IOutputPin *enable3;

        void configurePwmFrequency();
        
public:
        PwmGenerator(IOutputPin *_pwm1,
                     IOutputPin *_pwm2,
                     IOutputPin *_pwm3,
                     IOutputPin *_enable1,
                     IOutputPin *_enable2,
                     IOutputPin *_enable3);
        
        virtual ~PwmGenerator() {}

        void set(float duty_cycle1, float duty_cycle2, float duty_cycle3) override;
        void enable() override;
        void disable() override;
};

#endif // __PWM_GENERATOR_H
