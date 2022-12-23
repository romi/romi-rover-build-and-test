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

#ifndef __IARDUINO_H
#define __IARDUINO_H

#include <stdint.h>
#include "ArduinoConstants.h"

typedef void (*ISR)(void);

class IArduino
{
public:
        virtual ~IArduino() = default;

        virtual unsigned long micros() = 0;
        virtual void attachInterrupt(int pin, ISR isr, int mode) = 0;
        virtual void pinMode(uint8_t pin, uint8_t mode) = 0;
        virtual void digitalWrite(int pin, int high_low) = 0;
        virtual void delay(unsigned long ms) = 0;
};

#endif // __IARDUINO_H
