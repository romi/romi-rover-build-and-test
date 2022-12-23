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

#ifndef __IPWM_OUT_H
#define __IPWM_OUT_H

#include "ArduinoConstants.h"
#include "IArduino.h"
#include "IOutputPin.h"

#define PERX 255

class PwmOut : public IOutputPin
{
protected:
        int pin;
        
public:
        PwmOut(IArduino *_arduino, int _pin) : pin(_pin) {
                _arduino->pinMode(pin, OUTPUT);
        }
        
        virtual ~PwmOut() {}

        void set(float duty_cycle) override {
                // analogWrite(pin, (int) 255*duty_cycle);
                /*
                 * REG_TCC0_CCB0 - digital output D2 (Zero Pro/M0 Pro/M0 - digital pin D4)
                 * REG_TCC0_CCB1 - digital output D5
                 * REG_TCC0_CCB2 - digital output D6
                 * REG_TCC0_CCB3 - digital output D7
                 * REG_TCC1_CCB0 - digital output D4 (Zero Pro/M0 Pro/M0 - digital pin D2)
                 * REG_TCC1_CCB1 - digital output D3
                 * REG_TCC2_CCB0 - digital output D11
                 * REG_TCC2_CCB1 - digital output D13
                 */
                switch (pin) {
                case 2:
                        REG_TCC0_CCB0 = (int) (duty_cycle*PERX);
                        break;
                case 5:
                        REG_TCC0_CCB1 = (int) (duty_cycle*PERX);
                        break;
                case 6:
                        REG_TCC0_CCB2 = (int) (duty_cycle*PERX);
                        break;
                case 7:
                        REG_TCC0_CCB3 = (int) (duty_cycle*PERX);
                        break;
                case 4:
                        REG_TCC1_CCB0 = (int) (duty_cycle*PERX);
                        break;
                case 3:
                        REG_TCC1_CCB1 = (int) (duty_cycle*PERX);
                        break;
                case 11:
                        REG_TCC2_CCB0 = (int) (duty_cycle*PERX);
                        break;
                case 13:
                        REG_TCC2_CCB1 = (int) (duty_cycle*PERX);
                        break;
                }
        }
};

#endif // __IPWM_OUT_H
