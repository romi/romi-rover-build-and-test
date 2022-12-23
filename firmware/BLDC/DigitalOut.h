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

#ifndef __DIGITAL_OUT_H
#define __DIGITAL_OUT_H

#include "ArduinoConstants.h"
#include "IArduino.h"
#include "IOutputPin.h"

class DigitalOut : public IOutputPin
{
protected:
        IArduino *arduino;
        int pin;
        
public:
        DigitalOut(IArduino *_arduino, int _pin) : arduino(_arduino), pin(_pin) {
                arduino->pinMode(pin, OUTPUT);
        }
        
        virtual ~DigitalOut() {}

        void set(float value) override {
                if (value == 0.0f)
                        arduino->digitalWrite(pin, LOW);
                else 
                        arduino->digitalWrite(pin, HIGH);
        }
};

#endif // __DIGITAL_OUT_H
