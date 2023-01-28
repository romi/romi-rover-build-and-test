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

  You should have received a copy opositionf the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef __BLDC_H
#define __BLDC_H

#include "fixed.h"
#include "IBLDC.h"
#include "IArduino.h"
#include "IEncoder.h"
#include "IOutputPin.h"
#include "IPwmGenerator.h"

float normalize_angle(float angle);

class IMU;

class BLDC : public IBLDC
{
protected:
        IPwmGenerator *generator_;
        IOutputPin *sleep_pin_;
        IOutputPin *reset_pin_;
        float poles_;
        float power_;
        // float position_;
        float phase_absolute_;
        float phase_relative_;
                
        void set_phase(float value);
        // void incr_phase(float delta);
        
        float angle_to_phase(float angle);

public:
        BLDC(IPwmGenerator *generator,
             IOutputPin *sleep,
             IOutputPin *reset,
             uint8_t poles);
        
        virtual ~BLDC() override = default;

        void set_power(float p) override;
        float get_power() const override;
        
        void incr_position(float delta) override;
        
        void wakeup() override;
        void sleep() override;
        void reset() override;
};

#endif // __BLDC_H
