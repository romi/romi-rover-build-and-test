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

#ifndef __PWM_GENERATOR_H
#define __PWM_GENERATOR_H

#include "IOutputPin.h"
#include "IPwmGenerator.h"

#define SINE_TABLE_SIZE 1536

class PwmGenerator : public IPwmGenerator
{
protected:
        IOutputPin *pwm1;
        IOutputPin *pwm2;
        IOutputPin *pwm3;
        IOutputPin *enable1;
        IOutputPin *enable2;
        IOutputPin *enable3;
        float sineTable[SINE_TABLE_SIZE];
        float amplitude;
        
        void configurePwmFrequency();
        void initializeSineTable();
        
public:
        PwmGenerator(IOutputPin *_pwm1,
                     IOutputPin *_pwm2,
                     IOutputPin *_pwm3,
                     IOutputPin *_enable1,
                     IOutputPin *_enable2,
                     IOutputPin *_enable3);
        
        virtual ~PwmGenerator() {}

        /** The phase should be a normalized angle between 0 and 1. */
        void setPhase(float phase) override;
        
        /** The ampltide should be a a value between 0 and 1. */
        void setAmplitude(float p) override;
        
        void enable() override;
        void disable() override;
};

#endif // __PWM_GENERATOR_H
