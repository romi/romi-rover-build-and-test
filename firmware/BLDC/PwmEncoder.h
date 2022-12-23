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
#ifndef __PWM_ENCODER_H
#define __PWM_ENCODER_H

#include "IArduino.h"
#include "IEncoder.h"
#include <stdint.h>

class PwmEncoder;

void setPwmEncoder(PwmEncoder *encoder);
void PwmEncoderRise();
void PwmEncoderFall();

class PwmEncoder : public IEncoder
{
private:
        IArduino *arduino;
        int pin;
        unsigned long prevTime;
        uint16_t value;
        uint16_t minValue;
        uint16_t maxValue;
        float angle;
        
        friend void PwmEncoderRise();
        friend void PwmEncoderFall();
        
        void rise() {
                prevTime = arduino->micros();
                attachInterrupt(pin, PwmEncoderFall, FALLING);
        }
        
        void fall() {
                uint16_t tmp = arduino->micros() - prevTime;
                /** Filter the value using [min,max] and |delta| < 100
                 * because the reading is not always reliable.  */
                if (tmp >= minValue && tmp <= maxValue) {
                        // uint16_t delta = value - tmp;
                        // if (delta < 0)
                        //         delta = -delta;
                        // if (delta < 100)
                                value = tmp;
                }
                attachInterrupt(pin, PwmEncoderRise, RISING);
        }

public:
        PwmEncoder(IArduino *_arduino, int _pin,
                   uint16_t _minValue, uint16_t _maxValue)
                : arduino(_arduino), pin(_pin), 
                  value(0), prevTime(0), minValue(_minValue),
                  maxValue(_maxValue), angle(0.0f) {
                // Sets the global encoder, used by the interrupt
                // service routines.
                setPwmEncoder(this); // Hmmm...
                arduino->attachInterrupt(pin, PwmEncoderFall, FALLING);
        }
        
        
        virtual ~PwmEncoder() {}
        
        uint16_t getValue() override {
                return value;
        }

        float getAngle() override {
                return (float) (value - minValue) / (float) (maxValue - minValue);
        }
};

#endif // __PWM_ENCODER_H
